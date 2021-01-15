#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/Writer.h"
#include "Core/Thread/Acquire.h"
#include "Database/Compact/BlockFile.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const uint32_t c_version = 3;
const uint32_t c_maxBlockCount = 8192;

class BlockReadStream : public StreamStream
{
	T_RTTI_CLASS;

public:
	BlockReadStream(BlockFile* blockFile, IStream* stream, int endOffset)
	:	StreamStream(stream, endOffset)
	,	m_blockFile(blockFile)
	{
	}

	virtual ~BlockReadStream()
	{
		close();
	}

	virtual void close() override final
	{
		if (m_stream)
		{
			m_blockFile->returnReadStream(m_stream);
			m_stream = 0;
		}
	}

private:
	Ref< BlockFile > m_blockFile;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BlockReadStream", BlockReadStream, StreamStream)

class BlockWriteStream : public IStream
{
	T_RTTI_CLASS;

public:
	BlockWriteStream(BlockFile* blockFile, IStream* stream, BlockFile::Block& outBlock)
	:	m_blockFile(blockFile)
	,	m_stream(stream)
	,	m_outBlock(outBlock)
	{
		m_outBlock.offset = m_stream->tell();
		m_outBlock.size = 0;
	}

	virtual ~BlockWriteStream()
	{
		close();
	}

	virtual void close() override final
	{
		if (m_stream)
		{
			m_outBlock.size = m_stream->tell() - m_outBlock.offset;
			m_blockFile->needFlushTOC();
			m_stream = 0;
		}
	}

	virtual bool canRead() const override final
	{
		return false;
	}

	virtual bool canWrite() const override final
	{
		return true;
	}

	virtual bool canSeek() const override final
	{
		return false;
	}

	virtual int64_t tell() const override final
	{
		T_ASSERT(m_stream);
		return m_stream->tell() - m_outBlock.offset;
	}

	virtual int64_t available() const override final
	{
		return 0;
	}

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final
	{
		return 0;
	}

	virtual int64_t read(void* block, int64_t nbytes) override final
	{
		return 0;
	}

	virtual int64_t write(const void* block, int64_t nbytes) override final
	{
		T_ASSERT(m_stream);
		return m_stream->write(block, nbytes);
	}

	virtual void flush() override final
	{
		T_ASSERT(m_stream);
		m_stream->flush();
	}

private:
	Ref< BlockFile > m_blockFile;
	Ref< IStream > m_stream;
	BlockFile::Block& m_outBlock;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BlockWriteStream", BlockWriteStream, IStream)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BlockFile", BlockFile, Object)

BlockFile::BlockFile()
:	m_flushAlways(false)
,	m_needFlushTOC(false)
{
}

BlockFile::~BlockFile()
{
	T_ASSERT(!m_stream);
}

bool BlockFile::create(const Path& fileName, bool flushAlways)
{
	if (FileSystem::getInstance().exist(fileName))
	{
		if (!FileSystem::getInstance().remove(fileName))
			return false;
	}

	m_stream = FileSystem::getInstance().open(fileName, File::FmRead | File::FmWrite);
	if (!m_stream)
		return false;

	m_fileName = fileName;
	m_flushAlways = flushAlways;
	m_unusedReadStreams.push_back(m_stream);

	flushTOC();

	return true;
}

bool BlockFile::open(const Path& fileName, bool readOnly, bool flushAlways)
{
	if (!readOnly && !FileSystem::getInstance().exist(fileName))
		return false;

	m_stream = FileSystem::getInstance().open(fileName, readOnly ? File::FmRead | File::FmMapped : File::FmRead | File::FmWrite);
	if (!m_stream)
		return false;

	Reader reader(m_stream);

	uint32_t magic, version;
	reader >> magic;
	reader >> version;

	if (magic != 'TBLK' || version != c_version)
		return false;

	uint32_t blockCount;
	reader >> blockCount;

	m_blocks.resize(blockCount);
	for (uint32_t i = 0; i < blockCount; ++i)
	{
		Block& block = m_blocks[i];
		reader >> block.id;
		reader >> block.offset;
		reader >> block.size;
	}

	m_stream->seek(IStream::SeekSet, 3 * sizeof(uint32_t) + c_maxBlockCount * (sizeof(uint32_t) + sizeof(Block)));

	m_fileName = fileName;
	m_flushAlways = flushAlways;
	m_unusedReadStreams.push_back(m_stream);

	return true;
}

void BlockFile::close()
{
	if (m_stream)
	{
		if (m_needFlushTOC)
			flushTOC();

		for (auto stream : m_unusedReadStreams)
			stream->close();

		m_stream->close();

		m_unusedReadStreams.clear();
		m_stream = nullptr;
	}
}

uint32_t BlockFile::allocBlockId()
{
	uint32_t maxBlockId = 0;
	for (const auto& block : m_blocks)
		maxBlockId = std::max(maxBlockId, block.id);

	Block block;
	block.id = maxBlockId + 1;
	block.offset = 0;
	block.size = 0;
	m_blocks.push_back(block);

	return block.id;
}

void BlockFile::freeBlockId(uint32_t blockId)
{
	for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it)
	{
		if (it->id == blockId)
		{
			m_blocks.erase(it);
			break;
		}
	}
}

Ref< IStream > BlockFile::readBlock(uint32_t blockId)
{
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(), [=](const Block& block) { return block.id == blockId; });
	if (it == m_blocks.end())
		return nullptr;

	Ref< IStream > stream;

	// Pop unused read streams from cache.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (!m_unusedReadStreams.empty())
		{
			stream = m_unusedReadStreams.back();
			m_unusedReadStreams.pop_back();
		}
	}

	// No unused stream available; create new read stream.
	if (!stream)
	{
		stream = FileSystem::getInstance().open(m_fileName, File::FmRead);
		if (!stream)
			return nullptr;
	}

	if (stream->seek(IStream::SeekSet, it->offset) < 0)
		return nullptr;

	return new BlockReadStream(this, stream, it->offset + it->size);
}

Ref< IStream > BlockFile::writeBlock(uint32_t blockId)
{
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(), [=](const Block& block) { return block.id == blockId; });
	if (it == m_blocks.end())
		return nullptr;

	if (m_stream->seek(IStream::SeekEnd, 0) < 0)
		return nullptr;

	return new BlockWriteStream(this, m_stream, *it);
}

void BlockFile::needFlushTOC()
{
	if (!m_flushAlways)
		m_needFlushTOC = true;
	else
		flushTOC();
}

void BlockFile::flushTOC()
{
	m_stream->seek(IStream::SeekSet, 0);

	Writer writer(m_stream);

	writer << uint32_t('TBLK');
	writer << c_version;

	uint32_t blockCount = uint32_t(m_blocks.size());
	writer << blockCount;

	for (const auto& block : m_blocks)
	{
		writer << block.id;
		writer << block.offset;
		writer << block.size;
	}

	int64_t padSize = 3 * sizeof(uint32_t) + c_maxBlockCount * (sizeof(uint32_t) + sizeof(Block)) - m_stream->tell();
	for (int64_t i = 0; i < padSize; ++i)
	{
		uint8_t padDummy = 0x00;
		m_stream->write(&padDummy, 1);
	}

	m_needFlushTOC = false;
}

void BlockFile::returnReadStream(IStream* readStream)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_unusedReadStreams.push_back(readStream);
}

	}
}

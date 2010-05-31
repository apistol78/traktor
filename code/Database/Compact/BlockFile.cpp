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

struct BlockPred
{
	uint32_t m_id;

	BlockPred(uint32_t id)
	:	m_id(id)
	{
	}

	bool operator () (const BlockFile::Block& block) const
	{
		return block.id == m_id;
	}
};

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

	virtual void close()
	{
		T_ASSERT (m_stream);

		m_outBlock.size = m_stream->tell() - m_outBlock.offset;
		m_stream = 0;

		m_blockFile->flushTOC();
	}
	
	virtual bool canRead() const
	{
		return false;
	}

	virtual bool canWrite() const
	{
		return true;
	}

	virtual bool canSeek() const
	{
		return false;
	}

	virtual int tell() const
	{
		T_ASSERT (m_stream);
		return m_stream->tell() - m_outBlock.offset;
	}

	virtual int available() const
	{
		return 0;
	}

	virtual int seek(SeekOriginType origin, int offset)
	{
		return 0;
	}

	virtual int read(void* block, int nbytes)
	{
		return 0;
	}

	virtual int write(const void* block, int nbytes)
	{
		T_ASSERT (m_stream);
		return m_stream->write(block, nbytes);
	}

	virtual void flush()
	{
		T_ASSERT (m_stream);
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

BlockFile::~BlockFile()
{
	T_ASSERT (!m_stream);
}

bool BlockFile::create(const Path& fileName)
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

	flushTOC();

	return true;
}

bool BlockFile::open(const Path& fileName, bool readOnly)
{
	if (!readOnly && !FileSystem::getInstance().exist(fileName))
		return false;

	m_stream = FileSystem::getInstance().open(fileName, readOnly ? File::FmRead : File::FmRead | File::FmWrite);
	if (!m_stream)
		return false;

	Reader reader(m_stream);

	uint32_t magic, version;
	reader >> magic;
	reader >> version;

	if (magic != 'TBLK' || version != 1)
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

	m_stream->seek(IStream::SeekSet, 3 * sizeof(uint32_t) + 4096 * (sizeof(uint32_t) + sizeof(Block)));
	m_fileName = fileName;

	return true;
}

void BlockFile::close()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}
}

uint32_t BlockFile::allocBlockId()
{
	uint32_t maxBlockId = 0;
	for (std::vector< Block >::const_iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
		maxBlockId = std::max(maxBlockId, i->id);
	
	Block block;
	block.id = maxBlockId + 1;
	block.offset = 0;
	block.size = 0;
	m_blocks.push_back(block);

	return block.id;
}

void BlockFile::freeBlockId(uint32_t blockId)
{
	for (std::vector< Block >::iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		if (i->id == blockId)
		{
			m_blocks.erase(i);
			break;
		}
	}
}

Ref< IStream > BlockFile::readBlock(uint32_t blockId)
{
	std::vector< Block >::const_iterator it = std::find_if(m_blocks.begin(), m_blocks.end(), BlockPred(blockId));
	if (it == m_blocks.end())
		return 0;

	Ref< IStream > stream = FileSystem::getInstance().open(m_fileName, File::FmRead);
	if (!stream)
		return 0;

	if (stream->seek(IStream::SeekSet, it->offset) < 0)
		return 0;

	return new StreamStream(stream, it->offset + it->size);
}

Ref< IStream > BlockFile::writeBlock(uint32_t blockId)
{
	std::vector< Block >::iterator it = std::find_if(m_blocks.begin(), m_blocks.end(), BlockPred(blockId));
	if (it == m_blocks.end())
		return 0;

	if (m_stream->seek(IStream::SeekEnd, 0) < 0)
		return 0;

	return new BlockWriteStream(this, m_stream, *it);
}

void BlockFile::flushTOC()
{
	m_stream->seek(IStream::SeekSet, 0);

	Writer writer(m_stream);

	writer << uint32_t('TBLK');
	writer << uint32_t(1);

	uint32_t blockCount = uint32_t(m_blocks.size());
	writer << blockCount;

	for (std::vector< Block >::const_iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		writer << i->id;
		writer << i->offset;
		writer << i->size;
	}

	uint32_t padSize = 3 * sizeof(uint32_t) + 4096 * (sizeof(uint32_t) + sizeof(Block)) - m_stream->tell();
	for (uint32_t i = 0; i < padSize; ++i)
	{
		uint8_t padDummy = 0x00;
		m_stream->write(&padDummy, 1);
	}
}

	}
}

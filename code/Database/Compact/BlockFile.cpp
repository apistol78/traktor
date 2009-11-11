#include "Database/Compact/BlockFile.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Thread/Acquire.h"

#if defined(_DEBUG)
#define T_DEBUG_USAGE 1
#else
#define T_DEBUG_USAGE 0
#endif

#if T_DEBUG_USAGE
#include "Core/Io/DebugStream.h"
#include "Core/Log/Log.h"
#endif

namespace traktor
{
	namespace db
	{
		namespace
		{

class BlockReadStream : public Stream
{
	T_RTTI_CLASS(BlockReadStream)

public:
	BlockReadStream(Stream* stream, const BlockFile::Block& block, Mutex& lock)
	:	m_stream(stream)
	,	m_block(block)
	,	m_offset(0)
	,	m_lock(lock)
	{
	}

	virtual void close()
	{
		T_ASSERT (m_stream);
		m_stream = 0;
	}

	virtual bool canRead() const
	{
		return true;
	}

	virtual bool canWrite() const
	{
		return false;
	}

	virtual bool canSeek() const
	{
		return true;
	}

	virtual int tell() const
	{
		T_ASSERT (m_stream);
		return m_offset;
	}

	virtual int available() const
	{
		return m_block.size - m_offset;
	}

	virtual int seek(SeekOriginType origin, int offset)
	{
		switch (origin)
		{
		case SeekCurrent:
			m_offset += offset;
			break;

		case SeekEnd:
			m_offset = m_block.size - offset;
			break;

		case SeekSet:
			m_offset = offset;
			break;
		}

		if (m_offset < 0)
			m_offset = 0;
		if (m_offset >= m_block.size)
			m_offset = m_block.size - 1;

		return m_offset;
	}

	virtual int read(void* block, int nbytes)
	{
		T_ASSERT (m_stream);

		nbytes = std::min(available(), nbytes);

		Acquire< Mutex > __lock__(m_lock);

		m_stream->seek(Stream::SeekSet, m_block.offset + m_offset);
	
		int nread = m_stream->read(block, nbytes);
		if (nread > 0)
			m_offset += nread;

		return nread;
	}

	virtual int write(const void* block, int nbytes)
	{
		return 0;
	}

	virtual void flush()
	{
	}

private:
	Mutex& m_lock;
	Ref< Stream > m_stream;
	const BlockFile::Block& m_block;
	uint32_t m_offset;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BlockReadStream", BlockReadStream, Stream)

class BlockWriteStream : public Stream
{
	T_RTTI_CLASS(BlockWriteStream)

public:
	BlockWriteStream(BlockFile* blockFile, Stream* stream, BlockFile::Block& outBlock)
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
	Ref< Stream > m_stream;
	BlockFile::Block& m_outBlock;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BlockWriteStream", BlockWriteStream, Stream)

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

#if T_DEBUG_USAGE
	m_stream = gc_new< DebugStream >(m_stream);
#endif

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

#if T_DEBUG_USAGE
	m_stream = gc_new< DebugStream >(m_stream);
#endif

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

	m_stream->seek(Stream::SeekSet, 3 * sizeof(uint32_t) + 4096 * (sizeof(uint32_t) + sizeof(Block)));

	return true;
}

void BlockFile::close()
{
	if (m_stream)
	{
#if T_DEBUG_USAGE
		checked_type_cast< DebugStream* >(m_stream)->dump(log::info);
#endif

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

Ref< Stream > BlockFile::readBlock(uint32_t blockId)
{
	for (std::vector< Block >::const_iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		if (i->id == blockId)
			return gc_new< BufferedStream >(gc_new< BlockReadStream >(m_stream, cref(*i), ref(m_lock)));
	}
	return 0;
}

Ref< Stream > BlockFile::writeBlock(uint32_t blockId)
{
	for (std::vector< Block >::iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		if (i->id == blockId)
		{
			m_stream->seek(Stream::SeekEnd, 0);
			return gc_new< BlockWriteStream >(this, m_stream, ref(*i));
		}
	}
	return 0;
}

void BlockFile::flushTOC()
{
	m_stream->seek(Stream::SeekSet, 0);

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

#include <cstring>
extern "C"
{
	#include <lzf.h>
}
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace compress
	{

class InflateLzfImpl : public RefCountImpl< IRefCount >
{
public:
	InflateLzfImpl(IStream* stream, uint32_t blockSize)
	:	m_stream(stream)
	,	m_compressedBlock(blockSize + blockSize / 16 + 64 + 3)
	,	m_decompressedBuffer(blockSize)
	,	m_decompressedBufferSize(0)
	,	m_startPosition(stream->tell())
	,	m_position(m_startPosition)
	{
	}

	void close()
	{
		m_stream = 0;
	}

	int read(void* block, int nbytes)
	{
		uint8_t* top = static_cast< uint8_t* >(block);
		uint8_t* ptr = static_cast< uint8_t* >(block);

		// Copy from buffer.
		if (m_decompressedBufferSize > 0)
		{
			int32_t ncopy = std::min< int32_t >(m_decompressedBufferSize, nbytes);

			std::memcpy(ptr, &m_decompressedBuffer[0], ncopy);
			ptr += ncopy;

			if (ncopy < m_decompressedBufferSize)
				std::memmove(&m_decompressedBuffer[0], &m_decompressedBuffer[ncopy], m_decompressedBufferSize - ncopy);

			m_decompressedBufferSize -= ncopy;
			nbytes -= ncopy;
		}

		if (nbytes <= 0)
		{
			m_position += int32_t(ptr - top);
			return int32_t(ptr - top);
		}

		T_ASSERT (m_decompressedBufferSize == 0);

		// Decompress directly into destination if requested block size is larger than buffer;
		// and output is large enough to contain a bit of overhead.
		while (nbytes >= int(m_decompressedBuffer.size()))
		{
			uint32_t compressedBlockSize = 0;
			Reader(m_stream) >> compressedBlockSize;

			bool uncompressedBlock = (compressedBlockSize & 0x80000000UL) != 0;
			compressedBlockSize &= ~0x80000000UL;

			if (!compressedBlockSize)
				break;

			if (!uncompressedBlock)
			{
				if (compressedBlockSize > m_compressedBlock.size())
				{
					log::error << L"Compressed block size too large" << Endl;
					break;
				}

				if (m_stream->read(&m_compressedBlock[0], compressedBlockSize) != compressedBlockSize)
					break;

				uint32_t decompressLength = lzf_decompress(
					&m_compressedBlock[0],
					compressedBlockSize,
					ptr,
					nbytes
				);
				if (!decompressLength)
					break;

				T_ASSERT (decompressLength <= m_decompressedBuffer.size());

				ptr += decompressLength;
				nbytes -= decompressLength;
			}
			else
			{
				if (m_stream->read(ptr, compressedBlockSize) != compressedBlockSize)
					break;

				T_ASSERT (compressedBlockSize <= m_decompressedBuffer.size());

				ptr += compressedBlockSize;
				nbytes -= compressedBlockSize;
			}
		}

		// Decompress into buffer and copy required size into output.
		while (nbytes > 0)
		{
			uint32_t compressedBlockSize = 0;
			Reader(m_stream) >> compressedBlockSize;

			bool uncompressedBlock = (compressedBlockSize & 0x80000000UL) != 0;
			compressedBlockSize &= ~0x80000000UL;

			if (!compressedBlockSize)
				break;

			T_ASSERT (m_decompressedBufferSize == 0);

			if (!uncompressedBlock)
			{
				if (compressedBlockSize > m_compressedBlock.size())
				{
					log::error << L"Compressed block size too large" << Endl;
					break;
				}

				if (m_stream->read(&m_compressedBlock[0], compressedBlockSize) != compressedBlockSize)
					break;

				m_decompressedBufferSize = lzf_decompress(
					&m_compressedBlock[0],
					compressedBlockSize,
					&m_decompressedBuffer[0],
					uint32_t(m_decompressedBuffer.size())
				);
				if (!m_decompressedBufferSize)
				{
					log::error << L"Unexpected zero size block" << Endl;
					break;
				}
			}
			else
			{
				if (m_stream->read(&m_decompressedBuffer[0], compressedBlockSize) != compressedBlockSize)
					break;

				m_decompressedBufferSize = compressedBlockSize;
			}

			int32_t ncopy = std::min< int32_t >(m_decompressedBufferSize, nbytes);

			std::memcpy(ptr, &m_decompressedBuffer[0], ncopy);
			ptr += ncopy;

			if (ncopy < m_decompressedBufferSize)
				std::memmove(&m_decompressedBuffer[0], &m_decompressedBuffer[ncopy], m_decompressedBufferSize - ncopy);

			m_decompressedBufferSize -= ncopy;
			nbytes -= ncopy;
		}

		m_position += int32_t(ptr - top);
		return int32_t(ptr - top);
	}

	int setLogicalPosition(int position)
	{
		// Seeking backwards, restart from beginning.
		if (position < m_position)
		{
			m_stream->seek(IStream::SeekSet, m_startPosition);
			m_decompressedBufferSize = 0;
		}

		// Read dummy blocks until we're at the desired position.
		uint8_t dummy[1024];
		while (m_position < position)
		{
			int nread = read(dummy, std::min< int >(sizeof_array(dummy), position - m_position));
			if (nread <=  0)
				return -1;
		}

		return m_position;
	}

	int getLogicalPosition() const
	{
		return m_position;
	}

private:
	Ref< IStream > m_stream;
	AlignedVector< uint8_t > m_compressedBlock;
	AlignedVector< uint8_t > m_decompressedBuffer;
	int32_t m_decompressedBufferSize;
	int32_t m_startPosition;
	int32_t m_position;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.InflateStreamLzf", InflateStreamLzf, IStream)

InflateStreamLzf::InflateStreamLzf(IStream* stream, uint32_t blockSize)
:	m_impl(new InflateLzfImpl(stream, blockSize))
{
}

InflateStreamLzf::~InflateStreamLzf()
{
	close();
}

void InflateStreamLzf::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool InflateStreamLzf::canRead() const
{
	return true;
}

bool InflateStreamLzf::canWrite() const
{
	return false;
}

bool InflateStreamLzf::canSeek() const
{
	return true;
}

int InflateStreamLzf::tell() const
{
	return m_impl->getLogicalPosition();
}

int InflateStreamLzf::available() const
{
	T_FATAL_ERROR;
	return 0;
}

int InflateStreamLzf::seek(SeekOriginType origin, int offset)
{
	T_ASSERT_M (origin != SeekEnd, L"SeekEnd is not allowed");
	if (origin == SeekCurrent)
		offset += m_impl->getLogicalPosition();
	return m_impl->setLogicalPosition(offset);
}

int InflateStreamLzf::read(void* block, int nbytes)
{
	return m_impl->read(block, nbytes);
}

int InflateStreamLzf::write(const void* block, int nbytes)
{
	T_FATAL_ERROR;
	return 0;
}

void InflateStreamLzf::flush()
{
}

	}
}

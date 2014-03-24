#include <cstring>
#include <minilzo.h>
#include "Compress/Lzo/DeflateStreamLzo.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace compress
	{

class DeflateLzoImpl : public RefCountImpl< IRefCount >
{
public:
	DeflateLzoImpl(IStream* stream, uint32_t blockSize)
	:	m_stream(stream)
	,	m_uncompressedBuffer(blockSize)
	,	m_compressedBlock(blockSize + blockSize / 16 + 64 + 3)
	,	m_uncompressedBufferCount(0)
	,	m_workMemory(LZO1X_1_MEM_COMPRESS + sizeof(lzo_align_t))
	{
	}

	void close()
	{
		flush();
		m_stream = 0;
	}

	int write(const void* block, int nbytes)
	{
		const uint8_t* top = static_cast< const uint8_t* >(block);
		const uint8_t* ptr = static_cast< const uint8_t* >(block);

		while (nbytes > 0)
		{
			int32_t ncopy = std::min< int32_t >(nbytes, int32_t(m_uncompressedBuffer.size() - m_uncompressedBufferCount));
			std::memcpy(&m_uncompressedBuffer[m_uncompressedBufferCount], ptr, ncopy);
			m_uncompressedBufferCount += ncopy;
			ptr += ncopy;
			nbytes -= ncopy;

			if (m_uncompressedBufferCount >= m_uncompressedBuffer.size())
			{
				lzo_uint compressedBlockSize = m_uncompressedBuffer.size();
				lzo1x_1_compress(
					&m_uncompressedBuffer[0],
					m_uncompressedBufferCount,
					&m_compressedBlock[0],
					&compressedBlockSize,
					&m_workMemory[0]
				);

				// Write size of compressed block.
				Writer(m_stream) << uint32_t(compressedBlockSize);

				// Write content of compressed block.
				if (m_stream->write(&m_compressedBlock[0], int32_t(compressedBlockSize)) != compressedBlockSize)
					break;

				m_uncompressedBufferCount = 0;
			}
		}

		return int(ptr - top);
	}

	void flush()
	{
		if (m_uncompressedBufferCount > 0)
		{
			lzo_uint compressedBlockSize = m_uncompressedBuffer.size();
			lzo1x_1_compress(
				&m_uncompressedBuffer[0],
				m_uncompressedBufferCount,
				&m_compressedBlock[0],
				&compressedBlockSize,
				&m_workMemory[0]
			);

			// Write size of compressed block.
			Writer(m_stream) << uint32_t(compressedBlockSize);

			// Write content of compressed block.
			if (m_stream->write(&m_compressedBlock[0], int32_t(compressedBlockSize)) != compressedBlockSize)
				log::error << L"Failed to flush LZO stream; unable to write compressed block" << Endl;

			m_uncompressedBufferCount = 0;
		}
	}

private:
	Ref< IStream > m_stream;
	AlignedVector< uint8_t > m_uncompressedBuffer;
	AlignedVector< uint8_t > m_compressedBlock;
	AlignedVector< uint8_t > m_workMemory;
	uint32_t m_uncompressedBufferCount;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.DeflateStreamLzo", DeflateStreamLzo, IStream)

DeflateStreamLzo::DeflateStreamLzo(IStream* stream, uint32_t blockSize)
:	m_impl(new DeflateLzoImpl(stream, blockSize))
{
}

DeflateStreamLzo::~DeflateStreamLzo()
{
	close();
}

void DeflateStreamLzo::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool DeflateStreamLzo::canRead() const
{
	return false;
}

bool DeflateStreamLzo::canWrite() const
{
	return true;
}

bool DeflateStreamLzo::canSeek() const
{
	return false;
}

int DeflateStreamLzo::tell() const
{
	T_FATAL_ERROR;
	return 0;
}

int DeflateStreamLzo::available() const
{
	T_FATAL_ERROR;
	return 0;
}

int DeflateStreamLzo::seek(SeekOriginType origin, int offset)
{
	T_FATAL_ERROR;
	return 0;
}

int DeflateStreamLzo::read(void* block, int nbytes)
{
	T_FATAL_ERROR;
	return 0;
}

int DeflateStreamLzo::write(const void* block, int nbytes)
{
	T_ASSERT (m_impl);
	return m_impl->write(block, nbytes);
}

void DeflateStreamLzo::flush()
{
	T_ASSERT (m_impl);
	m_impl->flush();
}

	}
}

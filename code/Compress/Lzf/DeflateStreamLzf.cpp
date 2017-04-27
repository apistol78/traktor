/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
extern "C"
{
	#include <lzf.h>
}
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace compress
	{

class DeflateLzfImpl : public RefCountImpl< IRefCount >
{
public:
	DeflateLzfImpl(IStream* stream, uint32_t blockSize)
	:	m_stream(stream)
	,	m_uncompressedBuffer(blockSize)
	,	m_compressedBlock(blockSize - 1)
	,	m_uncompressedBufferCount(0)
	{
	}

	void close()
	{
		flush();
		m_stream = 0;
	}

	int64_t write(const void* block, int64_t nbytes)
	{
		const uint8_t* top = static_cast< const uint8_t* >(block);
		const uint8_t* ptr = static_cast< const uint8_t* >(block);

		while (nbytes > 0)
		{
			int64_t ncopy = std::min< int64_t >(nbytes, int64_t(m_uncompressedBuffer.size() - m_uncompressedBufferCount));
			std::memcpy(&m_uncompressedBuffer[m_uncompressedBufferCount], ptr, ncopy);
			m_uncompressedBufferCount += ncopy;
			ptr += ncopy;
			nbytes -= ncopy;

			if (m_uncompressedBufferCount >= m_uncompressedBuffer.size())
			{
				uint32_t compressedBlockSize = lzf_compress(
					&m_uncompressedBuffer[0],
					m_uncompressedBufferCount,
					&m_compressedBlock[0],
					uint32_t(m_compressedBlock.size())
				);
				if (compressedBlockSize)
				{
					// Write size of compressed block.
					Writer(m_stream) << uint32_t(compressedBlockSize);

					// Write content of compressed block.
					if (m_stream->write(&m_compressedBlock[0], compressedBlockSize) != compressedBlockSize)
						break;
				}
				else	// Unable to compress.
				{
					// Write size of uncompressed block.
					Writer(m_stream) << uint32_t(m_uncompressedBufferCount | 0x80000000UL);

					// Write content of uncompressed block.
					if (m_stream->write(&m_uncompressedBuffer[0], m_uncompressedBufferCount) != m_uncompressedBufferCount)
						break;
				}

				m_uncompressedBufferCount = 0;
			}
		}

		return int64_t(ptr - top);
	}

	void flush()
	{
		if (m_uncompressedBufferCount > 0)
		{
			uint32_t compressedBlockSize = lzf_compress(
				&m_uncompressedBuffer[0],
				m_uncompressedBufferCount,
				&m_compressedBlock[0],
				uint32_t(m_compressedBlock.size())
			);

			if (compressedBlockSize)
			{
				// Write size of compressed block.
				Writer(m_stream) << uint32_t(compressedBlockSize);

				// Write content of compressed block.
				if (m_stream->write(&m_compressedBlock[0], compressedBlockSize) != compressedBlockSize)
					log::error << L"Failed to flush LZF stream; unable to write compressed block" << Endl;
			}
			else	// Unable to compress.
			{
				// Write size of uncompressed block.
				Writer(m_stream) << uint32_t(m_uncompressedBufferCount | 0x80000000UL);

				// Write content of uncompressed block.
				if (m_stream->write(&m_uncompressedBuffer[0], m_uncompressedBufferCount) != m_uncompressedBufferCount)
					log::error << L"Failed to flush LZF stream; unable to write uncompressed block" << Endl;
			}

			m_uncompressedBufferCount = 0;
		}
	}

private:
	Ref< IStream > m_stream;
	AlignedVector< uint8_t > m_uncompressedBuffer;
	AlignedVector< uint8_t > m_compressedBlock;
	int64_t m_uncompressedBufferCount;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.DeflateStreamLzf", DeflateStreamLzf, IStream)

DeflateStreamLzf::DeflateStreamLzf(IStream* stream, uint32_t blockSize)
:	m_impl(new DeflateLzfImpl(stream, blockSize))
{
}

DeflateStreamLzf::~DeflateStreamLzf()
{
	close();
}

void DeflateStreamLzf::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool DeflateStreamLzf::canRead() const
{
	return false;
}

bool DeflateStreamLzf::canWrite() const
{
	return true;
}

bool DeflateStreamLzf::canSeek() const
{
	return false;
}

int64_t DeflateStreamLzf::tell() const
{
	T_FATAL_ERROR;
	return 0;
}

int64_t DeflateStreamLzf::available() const
{
	T_FATAL_ERROR;
	return 0;
}

int64_t DeflateStreamLzf::seek(SeekOriginType origin, int64_t offset)
{
	T_FATAL_ERROR;
	return 0;
}

int64_t DeflateStreamLzf::read(void* block, int64_t nbytes)
{
	T_FATAL_ERROR;
	return 0;
}

int64_t DeflateStreamLzf::write(const void* block, int64_t nbytes)
{
	T_ASSERT (m_impl);
	return m_impl->write(block, nbytes);
}

void DeflateStreamLzf::flush()
{
	T_ASSERT (m_impl);
	m_impl->flush();
}

	}
}

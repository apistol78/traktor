#include <cstring>
#include <vector>
#include <zlib.h>
#include "Zip/CompressedStream.h"

namespace traktor
{
	namespace zip
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.zip.CompressedStream", CompressedStream, Stream)

CompressedStream::CompressedStream(Stream* stream)
:	m_stream(stream)
,	m_bufferEnd(m_buffer)
{
	T_ASSERT (m_stream);
	T_ASSERT (m_stream->canRead() ^ m_stream->canWrite());
}

void CompressedStream::close()
{
	flush();
	m_stream->close();
}

bool CompressedStream::canRead() const
{
	return m_stream->canRead();
}

bool CompressedStream::canWrite() const
{
	return m_stream->canWrite();
}

bool CompressedStream::canSeek() const
{
	return false;
}

int CompressedStream::tell() const
{
	T_ASSERT (0);
	return 0;
}

int CompressedStream::available() const
{
	if (m_bufferEnd <= m_buffer)
	{
		int compressedBufferSize;
		m_stream->read(&compressedBufferSize, sizeof(compressedBufferSize));

		std::vector< char > compressedBuffer(compressedBufferSize);
		m_stream->read(&compressedBuffer[0], compressedBufferSize);

		int bufferSize = CompressionBufferSize;
		uncompress((Bytef*)m_buffer, (uLongf*)&bufferSize, (const Bytef*)&compressedBuffer[0], compressedBufferSize);

		m_bufferEnd = &m_buffer[bufferSize];
	}
	return int(m_bufferEnd - m_buffer);
}

int CompressedStream::seek(SeekOriginType origin, int offset)
{
	T_ASSERT (0);
	return 0;
}

int CompressedStream::read(void* block, int nbytes)
{
	char* out = static_cast< char* >(block);
	while (nbytes > 0)
	{
		int bufferCount = int(m_bufferEnd - m_buffer);
		if (bufferCount > 0)
		{
			int copyFromBuffer = std::min< int >(bufferCount, nbytes);
			if (copyFromBuffer > 0)
			{
				std::memcpy(out, m_buffer, copyFromBuffer);
				std::memmove(m_buffer, &m_buffer[copyFromBuffer], bufferCount - copyFromBuffer);
			}
			m_bufferEnd -= copyFromBuffer;
			nbytes -= copyFromBuffer;
			out += copyFromBuffer;
		}
		else
		{
			int compressedBufferSize;
			int nread = m_stream->read(&compressedBufferSize, sizeof(compressedBufferSize));
			if (nread != sizeof(compressedBufferSize))
				return 0;

			std::vector< char > compressedBuffer(compressedBufferSize);
			nread = m_stream->read(&compressedBuffer[0], compressedBufferSize);
			if (nread != compressedBufferSize)
				return 0;

			int bufferSize = CompressionBufferSize;
			uncompress((Bytef*)m_buffer, (uLongf*)&bufferSize, (const Bytef*)&compressedBuffer[0], compressedBufferSize);

			m_bufferEnd = &m_buffer[bufferSize];
		}
	}
	return int(out - static_cast< char* >(block));
}

int CompressedStream::write(const void* block, int nbytes)
{
	int written = 0;
	while (nbytes > 0)
	{
		int bufferCount = int(m_bufferEnd - m_buffer);
		int bufferAvail = CompressionBufferSize - bufferCount;
		if (bufferAvail > 0)
		{
			int copyIntoBuffer = std::min< int >(bufferAvail, nbytes);
			if (copyIntoBuffer > 0)
			{
				std::memcpy(m_bufferEnd, block, copyIntoBuffer);
				m_bufferEnd += copyIntoBuffer;
			}
			nbytes -= copyIntoBuffer;
			written += copyIntoBuffer;
		}
		else
		{
			flush();
		}
	}
	return written;
}

void CompressedStream::flush()
{
	if (m_stream->canWrite() == false)
		return;

	int bufferCount = int(m_bufferEnd - m_buffer);
	if (bufferCount == 0)
		return;

	// Compress buffer.
	int compressedBufferSize = compressBound(bufferCount);
	std::vector< char > compressedBuffer(compressedBufferSize);
	compress((Bytef*)&compressedBuffer[0], (uLongf*)&compressedBufferSize, (const Bytef*)m_buffer, bufferCount);

	// Write into stream, prepend with compressed buffer size.
	m_stream->write(&compressedBufferSize, sizeof(int));
	m_stream->write(&compressedBuffer[0], compressedBufferSize);
	m_stream->flush();

	// Reset buffer pointer.
	m_bufferEnd = m_buffer;
}

	}
}

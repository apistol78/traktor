#include <cstring>
#include <algorithm>
#include "Core/Io/BufferedStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.BufferedStream", BufferedStream, Stream)

BufferedStream::BufferedStream(Stream* stream, uint32_t internalBufferSize)
:	m_stream(stream)
,	m_internalBufferSize(internalBufferSize)
,	m_readBuf(0)
,	m_writeBuf(0)
{
	m_readBufCnt[0] =
	m_readBufCnt[1] = 0;
	m_writeBufCnt = 0;

	if (m_stream->canRead())
		m_readBuf = new uint8_t [m_internalBufferSize];
	if (m_stream->canWrite())
		m_writeBuf = new uint8_t [m_internalBufferSize];
}

BufferedStream::~BufferedStream()
{
	delete[] m_writeBuf;
	delete[] m_readBuf;
}

void BufferedStream::close()
{
	flush();
	m_stream->close();
}

bool BufferedStream::canRead() const
{
	return m_stream->canRead();
}

bool BufferedStream::canWrite() const
{
	return m_stream->canWrite();
}

bool BufferedStream::canSeek() const
{
	return m_stream->canSeek();
}

int BufferedStream::tell() const
{
	return m_stream->tell();
}

int BufferedStream::available() const
{
	return m_stream->available() + (m_readBufCnt[1] - m_readBufCnt[0]);
}

int BufferedStream::seek(SeekOriginType origin, int offset)
{
	if (m_stream->canRead())
	{
		switch (origin)
		{
		case SeekCurrent:
			{
				// Is new offset within buffered bytes then just decrease buffer.
				int readBufAvail = m_readBufCnt[1] - m_readBufCnt[0];
				if (offset > 0 && offset < readBufAvail)
				{
					m_readBufCnt[0] += offset;
					return offset;
				}
			}
			break;

		case SeekEnd:
			break;

		case SeekSet:
			{
				// Is new offset within buffered bytes then just decrease buffer.
				int currentPosition = m_stream->tell();
				int distance = offset - currentPosition;
				int readBufAvail = m_readBufCnt[1] - m_readBufCnt[0];
				if (distance > 0 && distance < readBufAvail)
				{
					m_readBufCnt[0] += distance;
					return offset;
				}
			}
			break;
		}

		m_readBufCnt[0] =
		m_readBufCnt[1] = 0;
	}
	if (m_stream->canWrite())
	{
		flush();
	}
	return m_stream->seek(origin, offset);
}

int BufferedStream::read(void* block, int nbytes)
{
	uint8_t* out = static_cast< uint8_t* >(block);
	uint8_t* end = out + nbytes;

	if (nbytes <= int(m_internalBufferSize))
	{
		// Read and copy until number of desired bytes read is meet.
		while (out < end)
		{
			if (m_readBufCnt[0] >= m_readBufCnt[1])
			{
				// Read into buffer.
				int nread = m_stream->read(m_readBuf, m_internalBufferSize);
				if (nread <= 0)
					break;
				m_readBufCnt[0] = 0;
				m_readBufCnt[1] = nread;
			}

			// Copy from read buffer into output buffer.
			int ncopy = std::min(int(end - out), m_readBufCnt[1] - m_readBufCnt[0]);
			T_ASSERT (ncopy > 0);

			std::memcpy(out, &m_readBuf[m_readBufCnt[0]], ncopy);

			m_readBufCnt[0] += ncopy;
			T_ASSERT (m_readBufCnt[0] <= m_readBufCnt[1]);

			out += ncopy;
		}
	}
	else
	{
		// Requested more bytes than our internal buffer, read directly from stream.
		if (m_readBufCnt[0] < m_readBufCnt[1])
		{
			int ncopy = m_readBufCnt[1] - m_readBufCnt[0];
			T_ASSERT (ncopy <= nbytes);

			std::memcpy(out, &m_readBuf[m_readBufCnt[0]], ncopy);

			m_readBufCnt[0] += ncopy;
			T_ASSERT (m_readBufCnt[0] <= m_readBufCnt[1]);

			out += ncopy;
		}

		int nread = m_stream->read(out, int(end - out));
		if (nread > 0)
			out += nread;
		else if (nread < 0 && out == block)
			return -1;
	}

	// Returned number of bytes copied.
	return static_cast< int >(out - static_cast< uint8_t* >(block));
}

int BufferedStream::write(const void* block, int nbytes)
{
	int nwritten = 0;

	if (nbytes < int(m_internalBufferSize))
	{
		const uint8_t* ptr = static_cast< const uint8_t* >(block);
		while (nbytes > 0)
		{
			int space = m_internalBufferSize - m_writeBufCnt;
			int nwrite = std::min< int >(space, nbytes);
			
			std::memcpy(&m_writeBuf[m_writeBufCnt], ptr, nwrite);
			
			m_writeBufCnt += nwrite;
			nwritten += nwrite;
			nbytes -= nwrite;
			ptr += nwrite;

			if (m_writeBufCnt >= int(m_internalBufferSize))
				flush();
		}
	}
	else
	{
		flush();
		nwritten = m_stream->write(block, nbytes);
	}

	return nwritten;
}

void BufferedStream::flush()
{
	if (m_writeBufCnt > 0)
	{
		m_stream->write(m_writeBuf, m_writeBufCnt);
		m_writeBufCnt = 0;
	}
	m_stream->flush();
}

}

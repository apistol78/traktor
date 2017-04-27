/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <algorithm>
#include "Core/Io/BufferedStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.BufferedStream", BufferedStream, IStream)

BufferedStream::BufferedStream(IStream* stream, uint32_t internalBufferSize)
:	m_stream(stream)
,	m_internalBufferSize(internalBufferSize)
,	m_readBuf(0)
,	m_writeBuf(0)
{
	m_readBufCnt[0] =
	m_readBufCnt[1] = 0;
	m_writeBufCnt = 0;

	if (m_stream->canRead())
		m_readBuf.reset(new uint8_t [m_internalBufferSize]);
	if (m_stream->canWrite())
		m_writeBuf.reset(new uint8_t [m_internalBufferSize]);
}

BufferedStream::BufferedStream(IStream* stream, const void* appendData, uint32_t appendDataSize, uint32_t internalBufferSize)
:	m_stream(stream)
,	m_internalBufferSize(internalBufferSize)
{
	T_ASSERT (appendData);
	T_ASSERT (appendDataSize <= internalBufferSize);

	m_readBufCnt[0] =
	m_readBufCnt[1] = 0;
	m_writeBufCnt = 0;

	if (m_stream->canRead())
	{
		m_readBuf.reset(new uint8_t [m_internalBufferSize]);
		std::memcpy(m_readBuf.ptr(), appendData, appendDataSize);
		m_readBufCnt[1] = appendDataSize;
	}
	if (m_stream->canWrite())
	{
		m_writeBuf.reset(new uint8_t [m_internalBufferSize]);
		std::memcpy(m_writeBuf.ptr(), appendData, appendDataSize);
		m_writeBufCnt = appendDataSize;
	}
}

void BufferedStream::close()
{
	if (m_stream)
	{
		flushWriteBuffer();
		m_stream->close();
		m_stream = 0;
	}
}

bool BufferedStream::canRead() const
{
	return m_stream ? m_stream->canRead() : false;
}

bool BufferedStream::canWrite() const
{
	return m_stream ? m_stream->canWrite() : false;
}

bool BufferedStream::canSeek() const
{
	return m_stream ? m_stream->canSeek() : false;
}

int64_t BufferedStream::tell() const
{
	if (!m_stream)
		return 0;
	else if (m_stream->canRead())
		return m_stream->tell() - m_readBufCnt[1] + m_readBufCnt[0];
	else if (m_stream->canWrite())
		return m_stream->tell() + m_writeBufCnt;
	else
		return m_stream->tell();
}

int64_t BufferedStream::available() const
{
	if (!m_stream)
		return 0;
	else if (m_stream->canRead())
		return m_stream->available() + (m_readBufCnt[1] - m_readBufCnt[0]);
	else
		return m_stream->available();
}

int64_t BufferedStream::seek(SeekOriginType origin, int64_t offset)
{
	if (!m_stream)
		return -1;

	if (m_stream->canRead())
	{
		int64_t readBufAvail = m_readBufCnt[1] - m_readBufCnt[0];
		switch (origin)
		{
		case SeekSet:
			offset -= tell();
			// Fall through

		case SeekCurrent:
			{
				if (offset > 0)
				{
					if (offset < readBufAvail)
					{
						m_readBufCnt[0] += offset;
						return offset;
					}
					else
					{
						m_readBufCnt[0] =
						m_readBufCnt[1] = 0;
						return m_stream->seek(SeekCurrent, offset - readBufAvail);
					}
				}
				else if (offset < 0)
				{
					int64_t position = tell() + offset;
					m_readBufCnt[0] =
					m_readBufCnt[1] = 0;
					return m_stream->seek(SeekSet, position);
				}
				else
					return 0;
			}
			break;

		case SeekEnd:
			m_readBufCnt[0] =
			m_readBufCnt[1] = 0;
			return m_stream->seek(SeekEnd, offset);

		default:
			break;
		}

		return -1;
	}
	else if (m_stream->canWrite())
	{
		flushWriteBuffer();
		return m_stream->seek(origin, offset);
	}
	else
		return -1;
}

int64_t BufferedStream::read(void* block, int64_t nbytes)
{
	if (!m_stream)
		return 0;

	uint8_t* out = static_cast< uint8_t* >(block);
	uint8_t* end = out + nbytes;

	if (nbytes <= int32_t(m_internalBufferSize))
	{
		// Read and copy until number of desired bytes read is meet.
		while (out < end)
		{
			if (m_readBufCnt[0] >= m_readBufCnt[1])
			{
				// Read into buffer.
				int64_t nread = m_stream->read(m_readBuf.ptr(), m_internalBufferSize);
				if (nread <= 0)
					break;
				m_readBufCnt[0] = 0;
				m_readBufCnt[1] = nread;
			}

			// Copy from read buffer into output buffer.
			int64_t ncopy = std::min(int64_t(end - out), m_readBufCnt[1] - m_readBufCnt[0]);
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
			int64_t ncopy = m_readBufCnt[1] - m_readBufCnt[0];
			T_ASSERT (ncopy <= nbytes);

			std::memcpy(out, &m_readBuf[m_readBufCnt[0]], ncopy);

			m_readBufCnt[0] += ncopy;
			T_ASSERT (m_readBufCnt[0] <= m_readBufCnt[1]);

			out += ncopy;
		}

		int64_t nread = m_stream->read(out, int64_t(end - out));
		if (nread > 0)
			out += nread;
		else if (nread < 0 && out == block)
			return -1;
	}

	// Returned number of bytes copied.
	return static_cast< int64_t >(out - static_cast< uint8_t* >(block));
}

int64_t BufferedStream::write(const void* block, int64_t nbytes)
{
	int64_t nwritten = 0;

	if (!m_stream)
		return 0;

	if (nbytes < int32_t(m_internalBufferSize))
	{
		const uint8_t* ptr = static_cast< const uint8_t* >(block);
		while (nbytes > 0)
		{
			int64_t space = m_internalBufferSize - m_writeBufCnt;
			int64_t nwrite = std::min< int64_t >(space, nbytes);
			
			std::memcpy(&m_writeBuf[m_writeBufCnt], ptr, nwrite);
			
			m_writeBufCnt += nwrite;
			nwritten += nwrite;
			nbytes -= nwrite;
			ptr += nwrite;

			if (m_writeBufCnt >= int64_t(m_internalBufferSize))
				flushWriteBuffer();
		}
	}
	else
	{
		flushWriteBuffer();

		const uint8_t* ptr = static_cast< const uint8_t* >(block);
		while (nbytes >= int64_t(m_internalBufferSize))
		{
			int64_t result = m_stream->write(ptr, m_internalBufferSize);
			if (result >= 0)
			{
				ptr += result;
				nbytes -= result;
				nwritten += result;
			}
			else
				return result;
		}

		T_ASSERT (nbytes < int64_t(m_internalBufferSize));
		if (nbytes > 0)
		{
			std::memcpy(m_writeBuf.ptr(), ptr, nbytes);
			m_writeBufCnt = nbytes;
			nwritten += nbytes;
		}
	}

	return nwritten;
}

void BufferedStream::flush()
{
	T_ASSERT (m_stream);
	flushWriteBuffer();
	m_stream->flush();
}

void BufferedStream::flushWriteBuffer()
{
	if (m_writeBufCnt > 0)
	{
		m_stream->write(m_writeBuf.ptr(), m_writeBufCnt);
		m_writeBufCnt = 0;
	}
}

}

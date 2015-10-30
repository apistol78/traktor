#include <algorithm>
#include <cstdio>
#include "Net/Http/HttpChunkStream.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpChunkStream", HttpChunkStream, IStream)

HttpChunkStream::HttpChunkStream(IStream* stream)
:	m_stream(stream)
,	m_available(-1)
{
	T_ASSERT (m_stream->canRead());
}

void HttpChunkStream::close()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}
}

bool HttpChunkStream::canRead() const
{
	return true;
}

bool HttpChunkStream::canWrite() const
{
	return false;
}

bool HttpChunkStream::canSeek() const
{
	return false;
}

int HttpChunkStream::tell() const
{
	return 0;
}

int HttpChunkStream::available() const
{
	if (m_available > 0)
		return m_available;

	return m_stream->available();
}

int HttpChunkStream::seek(SeekOriginType origin, int offset)
{
	return 0;
}

int HttpChunkStream::read(void* block, int nbytes)
{
	if (m_available == -1)
	{
		char buf[16];
		char* p = buf;

		while (p < &buf[15])
		{
			if (m_stream->read(p, 1) != 1)
				break;

			if (p == buf)
			{
				if (*p == '\r' || *p == '\n')
					continue;
			}

			if (p > buf)
			{
				if (p[-1] == '\r' && p[0] == '\n')
					break;
			}

			++p;
		}

		*p = '\0';
#if defined(_MSC_VER)
		sscanf_s(buf, "%x", &m_available);
#else
		std::sscanf(buf, "%x", &m_available);
#endif
	}

	if (m_available <= 0)
		return 0;

	int32_t nread = m_stream->read(block, std::min< int32_t >(nbytes, m_available));
	if (nread > 0)
	{
		m_available -= nread;
		if (m_available <= 0)
			m_available = -1;
	}

	return nread;
}

int HttpChunkStream::write(const void* block, int nbytes)
{
	return 0;
}

void HttpChunkStream::flush()
{
}

	}
}

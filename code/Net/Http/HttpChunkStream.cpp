/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

int64_t HttpChunkStream::tell() const
{
	return 0;
}

int64_t HttpChunkStream::available() const
{
	if (m_available > 0)
		return m_available;

	return m_stream->available();
}

int64_t HttpChunkStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t HttpChunkStream::read(void* block, int64_t nbytes)
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

	int64_t nread = m_stream->read(block, std::min< int64_t >(nbytes, m_available));
	if (nread > 0)
	{
		m_available -= nread;
		if (m_available <= 0)
			m_available = -1;
	}

	return nread;
}

int64_t HttpChunkStream::write(const void* block, int64_t nbytes)
{
	return 0;
}

void HttpChunkStream::flush()
{
}

	}
}

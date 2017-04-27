/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Io/StreamStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StreamStream", StreamStream, IStream);

StreamStream::StreamStream(IStream* stream, int64_t endOffset)
:	m_stream(stream)
,	m_startOffset(stream->tell())
,	m_endOffset(endOffset)
{
}

void StreamStream::close()
{
	T_ASSERT (m_stream);
	m_stream->close();
}

bool StreamStream::canRead() const
{
	T_ASSERT (m_stream);
	return m_stream->canRead();
}

bool StreamStream::canWrite() const
{
	T_ASSERT (m_stream);
	return m_stream->canWrite();
}

bool StreamStream::canSeek() const
{
	T_ASSERT (m_stream);
	return m_stream->canSeek();
}

int64_t StreamStream::tell() const
{
	T_ASSERT (m_stream);
	return m_stream->tell() - m_startOffset;
}

int64_t StreamStream::available() const
{
	T_ASSERT (m_stream);
	if (m_endOffset >= 0)
		return m_endOffset - m_stream->tell();
	else
		return m_stream->available();
}

int64_t StreamStream::seek(SeekOriginType origin, int64_t offset)
{
	T_ASSERT (m_stream);
	
	int64_t result = 0;
	switch (origin)
	{
	case SeekCurrent:
		result = m_stream->seek(SeekCurrent, offset);
		break;

	case SeekEnd:
		if (m_endOffset >= 0)
			result = m_stream->seek(SeekSet, m_endOffset + offset);
		else
			result = m_stream->seek(SeekEnd, offset);
		break;

	case SeekSet:
		result = m_stream->seek(SeekSet, m_startOffset + offset);
		break;
	}

	return result;
}

int64_t StreamStream::read(void* block, int64_t nbytes)
{
	T_ASSERT (m_stream);

	int64_t offset = m_stream->tell();
	nbytes = std::min(m_endOffset - offset, nbytes);

	return m_stream->read(block, nbytes);
}

int64_t StreamStream::write(const void* block, int64_t nbytes)
{
	T_ASSERT (m_stream);
	return m_stream->write(block, nbytes);
}

void StreamStream::flush()
{
	T_ASSERT (m_stream);
	m_stream->flush();
}

}

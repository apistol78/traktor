/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/IEncoding.h"
#include "Core/Io/IStream.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{

FileOutputStreamBuffer::FileOutputStreamBuffer(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
,	m_encodedSize(0)
{
}

void FileOutputStreamBuffer::close()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}
}

int32_t FileOutputStreamBuffer::overflow(const wchar_t* buffer, int32_t count)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (count > 0);

	uint32_t maxEncodedSize = count * IEncoding::MaxEncodingSize;
	if (maxEncodedSize > m_encodedSize)
	{
		m_encodedSize = maxEncodedSize;
		m_encoded.reset(new uint8_t [m_encodedSize]);
	}
	
	int32_t encodedCount = m_encoding->translate(buffer, count, m_encoded.ptr());
	if (encodedCount < 0)
		return -1;

	if (m_stream->write(m_encoded.c_ptr(), encodedCount) != encodedCount)
		return -1;

	return count;
}

}

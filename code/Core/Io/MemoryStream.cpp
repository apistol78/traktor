/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cstring>
#include "Core/Io/MemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.MemoryStream", MemoryStream, IStream);

MemoryStream::MemoryStream(void* buffer, int64_t bufferSize, bool readAllowed, bool writeAllowed, bool own)
:	m_buffer(static_cast< uint8_t* >(buffer))
,	m_bufferPtr(static_cast< uint8_t* >(buffer))
,	m_bufferSize(bufferSize)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
,	m_own(own)
{
}

MemoryStream::MemoryStream(const void* buffer, int64_t bufferSize)
:	m_buffer(static_cast< uint8_t* >(const_cast< void* >(buffer)))
,	m_bufferPtr(static_cast< uint8_t* >(const_cast< void* >(buffer)))
,	m_bufferSize(bufferSize)
,	m_readAllowed(true)
,	m_writeAllowed(false)
,	m_own(false)
{
}

MemoryStream::~MemoryStream()
{
	close();
}

void MemoryStream::close()
{
	if (m_own && m_buffer)
	{
		delete[] m_buffer;
		m_buffer = 0;
	}
}

bool MemoryStream::canRead() const
{
	return m_readAllowed;
}

bool MemoryStream::canWrite() const
{
	return m_writeAllowed;
}

bool MemoryStream::canSeek() const
{
	return true;
}

int64_t MemoryStream::tell() const
{
	return int64_t(m_bufferPtr - m_buffer);
}

int64_t MemoryStream::available() const
{
	return m_bufferSize - tell();
}

int64_t MemoryStream::seek(SeekOriginType origin, int64_t offset)
{
	switch (origin)
	{
	case SeekCurrent:
		m_bufferPtr += offset;
		break;
	case SeekEnd:
		m_bufferPtr = m_buffer + m_bufferSize + offset;
		break;
	case SeekSet:
		m_bufferPtr = m_buffer + offset;
		break;
	}
	if (m_bufferPtr < m_buffer)
		m_bufferPtr = m_buffer;
	else if (m_bufferPtr >= m_buffer + m_bufferSize)
		m_bufferPtr = m_buffer + m_bufferSize - 1;
	return tell();
}

int64_t MemoryStream::read(void* block, int64_t nbytes)
{
	if (!m_readAllowed)
		return 0;

	int64_t avail = std::min(nbytes, available());
	if (avail > 0)
	{
		std::memcpy(block, m_bufferPtr, avail);
		m_bufferPtr += avail;
	}

	return avail;
}

int64_t MemoryStream::write(const void* block, int64_t nbytes)
{
	if (!m_writeAllowed)
		return 0;

	int64_t space = std::min(nbytes, available());
	if (space > 0)
	{
		std::memcpy(m_bufferPtr, block, space);
		m_bufferPtr += space;
	}

	return space;
}

void MemoryStream::flush()
{
}

}

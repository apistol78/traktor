#include <cstring>
#include <algorithm>
#include "Core/Io/MemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.MemoryStream", MemoryStream, Stream)

MemoryStream::MemoryStream(void* buffer, uint32_t bufferSize, bool readAllowed, bool writeAllowed)
:	m_buffer(static_cast< uint8_t* >(buffer))
,	m_bufferPtr(static_cast< uint8_t* >(buffer))
,	m_bufferSize(bufferSize)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
{
}

MemoryStream::MemoryStream(const void* buffer, uint32_t bufferSize)
:	m_buffer(static_cast< uint8_t* >(const_cast< void* >(buffer)))
,	m_bufferPtr(static_cast< uint8_t* >(const_cast< void* >(buffer)))
,	m_bufferSize(bufferSize)
,	m_readAllowed(true)
,	m_writeAllowed(false)
{
}

void MemoryStream::close()
{
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

int MemoryStream::tell() const
{
	return int(m_bufferPtr - m_buffer);
}

int MemoryStream::available() const
{
	return m_bufferSize - tell();
}

int MemoryStream::seek(SeekOriginType origin, int offset)
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

int MemoryStream::read(void* block, int nbytes)
{
	if (!m_readAllowed)
		return 0;

	int avail = std::min(nbytes, available());
	if (avail > 0)
	{
		std::memcpy(block, m_bufferPtr, avail);
		m_bufferPtr += avail;
	}

	return avail;
}

int MemoryStream::write(const void* block, int nbytes)
{
	if (!m_writeAllowed)
		return 0;

	int space = std::min(nbytes, available());
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

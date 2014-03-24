#include <cstring>
#include "Core/Io/DynamicMemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DynamicMemoryStream", DynamicMemoryStream, IStream)

DynamicMemoryStream::DynamicMemoryStream(std::vector< uint8_t >& buffer, bool readAllowed, bool writeAllowed, const char* const name)
:	m_buffer(0)
,	m_readPosition(0)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
#if defined(_DEBUG)
,	m_name(name)
#endif
{
	m_buffer = &buffer;
}

DynamicMemoryStream::DynamicMemoryStream(bool readAllowed, bool writeAllowed, const char* const name)
:	m_buffer(0)
,	m_readPosition(0)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
#if defined(_DEBUG)
,	m_name(name)
#endif
{
	m_buffer = &m_internal;
}

void DynamicMemoryStream::close()
{
	m_readAllowed =
	m_writeAllowed = false;
	m_buffer = 0;
}

bool DynamicMemoryStream::canRead() const
{
	return m_readAllowed;
}

bool DynamicMemoryStream::canWrite() const
{
	return m_writeAllowed;
}

bool DynamicMemoryStream::canSeek() const
{
	return true;
}

int DynamicMemoryStream::tell() const
{
	if (m_readAllowed)
		return m_readPosition;
	else if (m_writeAllowed)
		return int(m_buffer->size());
	else
		return 0;
}

int DynamicMemoryStream::available() const
{
	return int(m_buffer->size() - m_readPosition);
}

int DynamicMemoryStream::seek(SeekOriginType origin, int offset)
{
	if (origin == SeekCurrent)
		m_readPosition += offset;
	else if (origin == SeekEnd)
		m_readPosition = uint32_t(m_buffer->size() + offset);
	else if (origin == SeekSet)
		m_readPosition = offset;
	return m_readPosition;
}

int DynamicMemoryStream::read(void* block, int nbytes)
{
	if (!m_readAllowed)
		return 0;

	if (nbytes <= 0)
		return 0;

	int nread = std::min< int >(nbytes, available());
	if (nread > 0)
	{
		std::memcpy(block, &(*m_buffer)[m_readPosition], nread);
		m_readPosition += nread;
	}

	return nread;
}

int DynamicMemoryStream::write(const void* block, int nbytes)
{
	if (!m_writeAllowed)
		return 0;

	if (nbytes <= 0)
		return 0;

	size_t org = m_buffer->size();
	m_buffer->resize(org + nbytes);

	std::memcpy(&(*m_buffer)[org], block, nbytes);
	return nbytes;
}

void DynamicMemoryStream::flush()
{
}

const std::vector< uint8_t >& DynamicMemoryStream::getBuffer() const
{
	return *m_buffer;
}

std::vector< uint8_t >& DynamicMemoryStream::getBuffer()
{
	return *m_buffer;
}

}


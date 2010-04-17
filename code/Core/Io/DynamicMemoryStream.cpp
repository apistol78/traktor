#include <cstring>
#include "Core/Io/DynamicMemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DynamicMemoryStream", DynamicMemoryStream, IStream)

DynamicMemoryStream::DynamicMemoryStream(std::vector< uint8_t >& buffer, bool readAllowed, bool writeAllowed)
:	m_buffer(buffer)
,	m_readPosition(0)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
{
}

DynamicMemoryStream::DynamicMemoryStream(bool readAllowed, bool writeAllowed)
:	m_buffer(m_internal)
,	m_readPosition(0)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
{
}

void DynamicMemoryStream::close()
{
	m_readAllowed =
	m_writeAllowed = false;
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
		return int(m_buffer.size());
	else
		return 0;
}

int DynamicMemoryStream::available() const
{
	return m_buffer.size() - m_readPosition;
}

int DynamicMemoryStream::seek(SeekOriginType origin, int offset)
{
	if (origin == SeekCurrent)
		m_readPosition += offset;
	else if (origin == SeekEnd)
		m_readPosition = m_buffer.size() + offset;
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
		std::memcpy(block, &m_buffer[m_readPosition], nread);
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

	size_t org = m_buffer.size();
	size_t nsz = org + nbytes;
	
	if (nsz > m_buffer.capacity())
		m_buffer.reserve(std::max< size_t >(nsz * 2, 128));
	
	m_buffer.resize(nsz);
	std::memcpy(&m_buffer[org], block, nbytes);

	return nbytes;
}

void DynamicMemoryStream::flush()
{
}

const std::vector< uint8_t >& DynamicMemoryStream::getBuffer() const
{
	return m_buffer;
}

}


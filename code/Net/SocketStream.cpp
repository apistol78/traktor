#include <algorithm>
#include <sstream>
#include "Core/Misc/SafeDestroy.h"
#include "Net/SocketStream.h"
#include "Net/Socket.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketStream", SocketStream, IStream)

SocketStream::SocketStream(Socket* socket, bool readAllowed, bool writeAllowed, int32_t timeout)
:	m_socket(socket)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
,	m_timeout(timeout)
,	m_offset(0)
{
}

void SocketStream::close()
{
	safeClose(m_socket);
}

bool SocketStream::canRead() const
{
	return bool(m_socket != nullptr) && m_readAllowed;
}

bool SocketStream::canWrite() const
{
	return bool(m_socket != nullptr) && m_writeAllowed;
}

bool SocketStream::canSeek() const
{
	return false;
}

int64_t SocketStream::tell() const
{
	return m_offset;
}

int64_t SocketStream::available() const
{
	unsigned long queued = 0;
	m_socket->ioctl(IccReadPending, &queued);
	return int64_t(queued);
}

int64_t SocketStream::seek(SeekOriginType origin, int64_t offset)
{
	switch (origin)
	{
	case SeekCurrent:
		m_offset += offset;
		break;

	case SeekEnd:
		m_offset = available() - offset;
		break;

	case SeekSet:
		m_offset = offset;
		break;
	}
	return 0;
}

int64_t SocketStream::read(void* block, int64_t nbytes)
{
	T_ASSERT(m_readAllowed);

	if (!m_socket)
		return -1;

	if (m_timeout >= 0)
	{
		int32_t result = m_socket->select(true, false, false, m_timeout);
		if (result == 0)
			return 0;
		else if (result < 0)
			return -1;
	}

	int32_t result = m_socket->recv(block, nbytes);
	if (result == 0)
		return 0;
	else if (result < 0)
		return -1;

	m_offset += result;
	return result;
}

int64_t SocketStream::write(const void* block, int64_t nbytes)
{
	T_ASSERT(m_writeAllowed);

	if (!m_socket)
		return -1;

	if (m_timeout >= 0)
	{
		int32_t result = m_socket->select(false, true, false, m_timeout);
		if (result == 0)
			return 0;
		else if (result < 0)
			return -1;
	}

	int32_t result = m_socket->send(block, nbytes);
	if (result == 0)
		return 0;
	else if (result < 0)
		return -1;

	m_offset += result;
	return result;
}

void SocketStream::flush()
{
}

void SocketStream::setAccess(bool readAllowed, bool writeAllowed)
{
	m_readAllowed = readAllowed;
	m_writeAllowed = writeAllowed;
}

void SocketStream::setTimeout(int32_t timeout)
{
	m_timeout = timeout;
}

	}
}

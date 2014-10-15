#include <sstream>
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
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
}

bool SocketStream::canRead() const
{
	return bool(m_socket != 0) && m_readAllowed;
}

bool SocketStream::canWrite() const
{
	return bool(m_socket != 0) && m_writeAllowed;
}

bool SocketStream::canSeek() const
{
	return false;
}

int SocketStream::tell() const
{
	return m_offset;
}

int SocketStream::available() const
{
	unsigned long queued = 0;
	m_socket->ioctl(IccReadPending, &queued);
	return int(queued);
}

int SocketStream::seek(SeekOriginType origin, int offset)
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

int SocketStream::read(void* block, int nbytes)
{
	T_ASSERT (m_readAllowed);

	if (!m_socket)
		return -1;

	int32_t nread = 0;
	while (nread < nbytes)
	{
		if (m_timeout >= 0)
		{
			if (m_socket->select(true, false, false, m_timeout) <= 0)
			{
				if (nread <= 0)
					nread = -1;
				break;
			}
		}

		int32_t result = m_socket->recv((char*)block + nread, nbytes - nread);
		if (result <= 0)
		{
			if (nread <= 0)
				nread = result;
			break;
		}

		nread += result;
	}

	if (nread > 0)
		m_offset += nread;

	T_ASSERT (nread <= nbytes);
	return nread;
}

int SocketStream::write(const void* block, int nbytes)
{
	T_ASSERT (m_writeAllowed);

	if (!m_socket)
		return -1;

	int32_t nwritten = 0;
	while (nwritten < nbytes)
	{
		if (m_timeout >= 0)
		{
			if (m_socket->select(false, true, false, m_timeout) <= 0)
			{
				if (nwritten <= 0)
					nwritten = -1;
				break;
			}
		}

		int32_t result = m_socket->send((char*)block + nwritten, nbytes - nwritten);
		if (result < 0)
		{
			if (nwritten <= 0)
				nwritten = result;
			break;
		}

		nwritten += result;
	}

	m_offset += nwritten;

	T_ASSERT (nwritten <= nbytes);
	return nwritten;
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

#include "Avalanche/Protocol.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Client/ClientGetStream.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.ClientGetStream", ClientGetStream, IStream)

ClientGetStream::ClientGetStream(Client* client, net::TcpSocket* socket, int64_t blobSize)
:	m_client(client)
,	m_socket(socket)
,	m_blobSize(blobSize)
,	m_offset(0)
{
}

void ClientGetStream::close()
{
	uint8_t dummy[1024];

	// Ensure entire stream is consumed.
	while (m_socket != nullptr && m_offset < m_blobSize)
		read(dummy, 1024);

	// Return socket to be reused.
	if (m_socket)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_client->m_lock);
		m_client->m_sockets.push_back(m_socket);
		m_socket = nullptr;
	}
}

bool ClientGetStream::canRead() const
{
	return true;
}

bool ClientGetStream::canWrite() const
{
	return false;
}

bool ClientGetStream::canSeek() const
{
	return false;
}

int64_t ClientGetStream::tell() const
{
	return 0;
}

int64_t ClientGetStream::available() const
{
	return m_blobSize - m_offset;
}

int64_t ClientGetStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t ClientGetStream::read(void* block, int64_t nbytes)
{
	if (!m_socket)
		return -1;

	if (nbytes <= 0)
		return 0;

	uint8_t* top = (uint8_t*)block;
	uint8_t* end = top + nbytes;
	uint8_t* ptr = top;

	while (ptr < end)
	{
		int64_t nread = (int64_t)(end - ptr);
		nread = std::min(m_blobSize - m_offset, nread);
		if (nread <= 0)
			break;
		T_FATAL_ASSERT(ptr + nread <= end);

		int64_t ngot = m_socket->recv(ptr, (int32_t)nread);
		if (ngot <= 0)
		{
			m_socket = nullptr;
			break;
		}
		T_FATAL_ASSERT(ngot <= nread);
		T_FATAL_ASSERT(ptr + ngot <= end);

		ptr += ngot;
		m_offset += ngot;
	}

	return (int64_t)(ptr - top);
}

int64_t ClientGetStream::write(const void* block, int64_t nbytes)
{
	return -1;
}

void ClientGetStream::flush()
{
}

	}
}

#include "Avalanche/Protocol.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Client/ClientPutStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Thread/Acquire.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.ClientPutStream", ClientPutStream, IStream)

ClientPutStream::ClientPutStream(Client* client, net::TcpSocket* socket)
:	m_client(client)
,	m_socket(socket)
{
}

void ClientPutStream::close()
{
	if (!m_socket)
		return;

	net::SocketStream socketStream(m_socket, true, true);
	Reader socketReader(&socketStream);
	Writer socketWriter(&socketStream);

	socketWriter << (uint8_t)c_subCommandPutCommit;

	uint8_t reply = 0;
	socketReader >> reply;
	if (reply == c_replyOk)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_client->m_lock);
		m_client->m_sockets.push_back(m_socket);
	}

	m_socket = nullptr;
}

bool ClientPutStream::canRead() const
{
	return false;
}

bool ClientPutStream::canWrite() const
{
	return true;
}

bool ClientPutStream::canSeek() const
{
	return false;
}

int64_t ClientPutStream::tell() const
{
	return 0;
}

int64_t ClientPutStream::available() const
{
	return 0;
}

int64_t ClientPutStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t ClientPutStream::read(void* block, int64_t nbytes)
{
	return -1;
}

int64_t ClientPutStream::write(const void* block, int64_t nbytes)
{
	if (!m_socket)
		return -1;

	if (nbytes <= 0)
		return 0;

	net::SocketStream socketStream(m_socket, true, true);
	Reader socketReader(&socketStream);
	Writer socketWriter(&socketStream);

	socketWriter << (uint8_t)c_subCommandPutAppend;
	socketWriter << (int64_t)nbytes;
	socketWriter.write(block, nbytes);

	uint8_t reply = 0;
	socketReader >> reply;
	if (reply != c_replyOk)
	{
		m_socket = nullptr;
		return -1;
	}

	return nbytes;
}

void ClientPutStream::flush()
{
}

	}
}

#include "Core/Thread/Acquire.h"
#include "Avalanche/Protocol.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Client/ClientGetStream.h"
#include "Avalanche/Client/ClientPutStream.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Client", Client, Object)

Client::Client(const net::SocketAddressIPv4& serverAddress)
:	m_serverAddress(serverAddress)
{
}

void Client::destroy()
{
	m_sockets.clear();
}

bool Client::ping()
{
	Ref< net::TcpSocket > socket = establish(c_commandPing);
	if (!socket)
		return false;

	net::SocketStream socketStream(socket, true, true);
	if (socketStream.write(&c_commandPing, sizeof(c_commandPing)) != sizeof(c_commandPing))
		return false;

	uint8_t reply = 0;
	if (socketStream.read(&reply, sizeof(reply)) != sizeof(reply))
		return false;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_sockets.push_back(socket);
	}

	return reply == c_replyOk;
}

bool Client::have(const Key& key)
{
	Ref< net::TcpSocket > socket = establish(c_commandStat);
	if (!socket)
		return false;

	net::SocketStream socketStream(socket, true, true);
	if (!key.write(&socketStream))
		return false;

	uint8_t reply = 0;
	if (socketStream.read(&reply, sizeof(reply)) != sizeof(reply))
		return false;

	if (reply == c_replyOk)
	{
		int64_t blobSize = 0;
		if (socketStream.read(&blobSize, sizeof(blobSize)) != sizeof(blobSize))
			return false;
	}

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_sockets.push_back(socket);
	}

	return reply == c_replyOk;
}

Ref< IStream > Client::get(const Key& key)
{
	Ref< net::TcpSocket > socket = establish(c_commandGet);
	if (!socket)
		return nullptr;

	net::SocketStream socketStream(socket, true, true);
	if (!key.write(&socketStream))
		return nullptr;

	uint8_t reply = 0;
	if (socketStream.read(&reply, sizeof(reply)) != sizeof(reply))
		return nullptr;

	if (reply != c_replyOk)
	{
		if (reply == c_replyFailure)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_sockets.push_back(socket);
		}
		return nullptr;
	}

	int64_t blobSize = 0;
	if (socketStream.read(&blobSize, sizeof(blobSize)) != sizeof(blobSize))
		return nullptr;

	return new ClientGetStream(this, socket, blobSize);
}

Ref< IStream > Client::put(const Key& key)
{
	Ref< net::TcpSocket > socket = establish(c_commandPut);
	if (!socket)
		return nullptr;

	net::SocketStream socketStream(socket, true, true);
	if (!key.write(&socketStream))
		return nullptr;

	uint8_t reply = 0;
	if (socketStream.read(&reply, sizeof(reply)) != sizeof(reply))
		return nullptr;

	if (reply != c_replyOk)
	{
		if (reply == c_replyFailure)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_sockets.push_back(socket);
		}
		return nullptr;
	}

	return new ClientPutStream(this, socket);
}

Ref< net::TcpSocket > Client::establish(uint8_t command)
{
	for (;;)
	{
		Ref< net::TcpSocket > socket;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (m_sockets.empty())
				break;
			socket = m_sockets.front();
			m_sockets.pop_front();
		}
		T_ASSERT(socket != nullptr);

		if (socket->select(false, true, false, 1000) >= 1)
		{
			if (socket->send(command) == 1)
				return socket;
		}
	}

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(m_serverAddress))
		return nullptr;
	if (socket->send(command) != 1)
		return nullptr;

	return socket;
}

	}
}

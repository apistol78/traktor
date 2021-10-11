#include "Core/Log/Log.h"
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
	m_streams.clear();
}

bool Client::ping()
{
	Ref< net::SocketStream > stream = establish(c_commandPing);
	if (!stream)
		return false;

	uint8_t reply = 0;
	if (stream->read(&reply, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		log::error << L"Unable to read reply from server (ping)." << Endl;
		return false;
	}

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_streams.push_back(stream);
	}

	return reply == c_replyOk;
}

bool Client::have(const Key& key)
{
	Ref< net::SocketStream > stream  = establish(c_commandStat);
	if (!stream)
		return false;

	if (!key.write(stream))
	{
		log::error << L"Unable to write key to server (have)." << Endl;
		return false;
	}

	uint8_t reply = 0;
	if (stream->read(&reply, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		log::error << L"Unable to read reply from server (have)." << Endl;
		return false;
	}

	if (reply == c_replyOk)
	{
		int64_t blobSize = 0;
		if (stream->read(&blobSize, sizeof(int64_t)) != sizeof(int64_t))
		{
			log::error << L"Unable to read blob size from server (have)." << Endl;
			return false;
		}
	}

	if (reply == c_replyOk || reply == c_replyFailure)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_streams.push_back(stream);
	}

	return reply == c_replyOk;
}

Ref< IStream > Client::get(const Key& key)
{
	Ref< net::SocketStream > stream = establish(c_commandGet);
	if (!stream)
		return nullptr;

	if (!key.write(stream))
	{
		log::error << L"Unable to write key to server (get)." << Endl;
		return nullptr;
	}

	uint8_t reply = 0;
	if (stream->read(&reply, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		log::error << L"Unable to read reply from server (get)." << Endl;
		return nullptr;
	}

	if (reply != c_replyOk)
	{
		if (reply == c_replyFailure)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_streams.push_back(stream);
		}
		return nullptr;
	}

	int64_t blobSize = 0;
	if (stream->read(&blobSize, sizeof(int64_t)) != sizeof(int64_t))
	{
		log::error << L"Unable to read blob size from server (get)." << Endl;
		return nullptr;
	}

	return new ClientGetStream(this, stream, blobSize);
}

Ref< IStream > Client::put(const Key& key)
{
	Ref< net::SocketStream > stream = establish(c_commandPut);
	if (!stream)
		return nullptr;

	if (!key.write(stream))
	{
		log::error << L"Unable to write key to server (put)." << Endl;
		return nullptr;
	}

	uint8_t reply = 0;
	if (stream->read(&reply, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		log::error << L"Unable to read reply from server (put)." << Endl;
		return nullptr;
	}

	if (reply != c_replyOk)
	{
		if (reply == c_replyFailure)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_streams.push_back(stream);
		}
		return nullptr;
	}

	return new ClientPutStream(this, stream);
}

bool Client::stats(Dictionary::Stats& outStats)
{
	Ref< net::SocketStream > stream = establish(c_commandStats);
	if (!stream)
		return false;

	if (stream->read(&outStats.blobCount, sizeof(uint32_t)) != sizeof(uint32_t))
		return false;
	if (stream->read(&outStats.memoryUsage, sizeof(uint64_t)) != sizeof(uint64_t))
		return false;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_streams.push_back(stream);
	}
	return true;
}

bool Client::getKeys(AlignedVector< Key >& outKeys)
{
	Ref< net::SocketStream > stream = establish(c_commandKeys);
	if (!stream)
		return false;

	uint64_t nkeys;
	if (stream->read(&nkeys, sizeof(uint64_t)) != sizeof(uint64_t))
		return false;

	outKeys.resize(nkeys);
	for (uint64_t i = 0; i < nkeys; ++i)
	{
		outKeys[i] = Key::read(stream);
		if (!outKeys[i].valid())
			return false;
	}

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_streams.push_back(stream);
	}
	return true;
}

Ref< net::SocketStream > Client::establish(uint8_t command)
{
	for (;;)
	{
		Ref< net::SocketStream > stream;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (m_streams.empty())
				break;
			stream = m_streams.front();
			m_streams.pop_front();
		}
		T_ASSERT(stream != nullptr);

		if (stream->write(&command, sizeof(uint8_t)) == sizeof(uint8_t))
			return stream;
	}

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(m_serverAddress))
	{
		log::error << L"Unable to connect to server at " << m_serverAddress.getHostName() << L":" << m_serverAddress.getPort() << L"." << Endl;
		return nullptr;
	}

	Ref< net::SocketStream > stream = new net::SocketStream(socket, true, true, 5000);
	if (stream->write(&command, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		log::error << L"Unable to write command to server." << Endl;
		return nullptr;
	}

	return stream;
}

	}
}

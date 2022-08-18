#include "Core/Log/Log.h"
#include "Database/Remote/IMessage.h"
#include "Database/Remote/Server/BusMessageListener.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/ConnectionMessageListener.h"
#include "Database/Remote/Server/DatabaseMessageListener.h"
#include "Database/Remote/Server/GroupMessageListener.h"
#include "Database/Remote/Server/InstanceMessageListener.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/StreamServer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Connection", Connection, Object)

Connection::Connection(
	Semaphore& connectionStringsLock,
	const SmallMap< std::wstring, std::wstring >& connectionStrings,
	net::StreamServer* streamServer,
	net::TcpSocket* clientSocket
)
:	m_streamServer(streamServer)
,	m_clientSocket(clientSocket)
,	m_nextHandle(1)
{
	m_transport = new net::BidirectionalObjectTransport(clientSocket);

	m_messageListeners.push_back(new BusMessageListener(this));
	m_messageListeners.push_back(new ConnectionMessageListener(this));
	m_messageListeners.push_back(new DatabaseMessageListener(connectionStringsLock, connectionStrings, m_streamServer->getListenPort(), this));
	m_messageListeners.push_back(new GroupMessageListener(this));
	m_messageListeners.push_back(new InstanceMessageListener(this));
}

void Connection::destroy()
{
	if (m_transport)
		m_transport = nullptr;

	if (m_clientSocket)
	{
		m_clientSocket->close();
		m_clientSocket = nullptr;
	}

	m_objectStore.clear();
}

bool Connection::process()
{
	if (!m_transport)
		return false;

	Ref< IMessage > message;
	if (m_transport->recv< IMessage >(100, message) == net::BidirectionalObjectTransport::Result::Disconnected)
		return false;

	if (!message)
		return false;

	for (auto listener : m_messageListeners)
	{
		if (listener->notify(message))
		{
			message = nullptr;
			break;
		}
	}

	if (message)
	{
		log::error << L"Unhandled message \"" << type_name(message) << L"\"; connection terminated." << Endl;
		return false;
	}

	return true;
}

void Connection::sendReply(const IMessage& message)
{
	if (!m_transport->send(&message))
	{
		log::error << L"Unable to send reply (" << type_name(&message) << L"); connection terminated." << Endl;
		destroy();
	}
}

uint32_t Connection::putObject(Object* object)
{
	uint32_t handle = m_nextHandle++;
	m_objectStore[handle] = object;
	return handle;
}

Object* Connection::getObject(uint32_t handle)
{
	auto it = m_objectStore.find(handle);
	return it != m_objectStore.end() ? it->second : nullptr;
}

void Connection::releaseObject(uint32_t handle)
{
	m_objectStore.remove(handle);
}

void Connection::setDatabase(IProviderDatabase* database)
{
	m_database = database;
}

IProviderDatabase* Connection::getDatabase() const
{
	return m_database;
}

net::StreamServer* Connection::getStreamServer() const
{
	return m_streamServer;
}

net::TcpSocket* Connection::getSocket() const
{
	return m_clientSocket;
}

	}
}

#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/BusMessageListener.h"
#include "Database/Remote/Server/ConnectionMessageListener.h"
#include "Database/Remote/Server/DatabaseMessageListener.h"
#include "Database/Remote/Server/GroupMessageListener.h"
#include "Database/Remote/Server/InstanceMessageListener.h"
#include "Database/Remote/Server/StreamMessageListener.h"
#include "Database/Remote/MessageTransport.h"
#include "Database/Remote/IMessage.h"
#include "Net/TcpSocket.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Connection", Connection, Object)

Connection::Connection(const Configuration* configuration, net::TcpSocket* clientSocket)
:	m_configuration(configuration)
,	m_clientSocket(clientSocket)
,	m_nextHandle(1)
{
	m_messageTransport = gc_new< MessageTransport >(clientSocket);

	m_messageListeners.push_back(gc_new< BusMessageListener >(this));
	m_messageListeners.push_back(gc_new< ConnectionMessageListener >(this));
	m_messageListeners.push_back(gc_new< DatabaseMessageListener >(m_configuration, this));
	m_messageListeners.push_back(gc_new< GroupMessageListener >(this));
	m_messageListeners.push_back(gc_new< InstanceMessageListener >(this));
	m_messageListeners.push_back(gc_new< StreamMessageListener >(this));

	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &Connection::messageThread), L"Message thread");
	T_ASSERT (m_thread);

	m_thread->start();
}

void Connection::destroy()
{
	if (m_thread)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = 0;
	}

	if (m_messageTransport)
		m_messageTransport = 0;

	if (m_clientSocket)
	{
		m_clientSocket->close();
		m_clientSocket = 0;
	}

	m_objectStore.clear();
}

bool Connection::alive() const
{
	if (!m_thread)
		return false;

	if (m_thread->wait(0))
		return false;

	return true;
}

void Connection::sendReply(const IMessage& message)
{
	if (!m_messageTransport->send(&message))
	{
		log::error << L"Unable to send reply (" << type_name(&message) << L"); connection terminated" << Endl;
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
	std::map< uint32_t, Ref< Object > >::iterator i = m_objectStore.find(handle);
	return i != m_objectStore.end() ? i->second : 0;
}

void Connection::releaseObject(uint32_t handle)
{
	m_objectStore.erase(handle);
}

void Connection::setDatabase(IProviderDatabase* database)
{
	m_database = database;
}

IProviderDatabase* Connection::getDatabase() const
{
	return m_database;
}

void Connection::messageThread()
{
	while (!m_thread->stopped())
	{
		Ref< IMessage > message;
		if (!m_messageTransport->receive(100, message))
		{
			log::info << L"Transport disconnected; connection terminated" << Endl;
			break;
		}

		if (!message)
			continue;

		for (RefArray< IMessageListener >::iterator i = m_messageListeners.begin(); i != m_messageListeners.end(); ++i)
		{
			if ((*i)->notify(message))
			{
				message = 0;
				break;
			}
		}

		if (message)
		{
			log::error << L"Unhandled message \"" << type_name(message) << L"\"; connection terminated" << Endl;
			break;
		}
	}
}

	}
}

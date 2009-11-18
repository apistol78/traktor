#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/MessageTransport.h"
#include "Net/TcpSocket.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Connection", Connection, Object)

Connection::Connection(net::TcpSocket* socket)
:	m_socket(socket)
{
	m_messageTransport = new MessageTransport(m_socket);
}

void Connection::destroy()
{
	if (m_messageTransport)
		m_messageTransport = 0;

	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
}

Ref< IMessage > Connection::sendMessage(const IMessage& message)
{
	Acquire< Semaphore > scopeLock(m_messageLock);
	Ref< IMessage > reply;

	if (!m_messageTransport)
		return 0;

	if (!m_messageTransport->send(&message))
		return 0;

#if !defined(_DEBUG)
	if (!m_messageTransport->receive(10000, reply))
		return false;
#else
	if (!m_messageTransport->receive(10000000, reply))
		return false;
#endif

	return reply;
}

	}
}

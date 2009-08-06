#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/MessageTransport.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Connection", Connection, Object)

Connection::Connection(net::TcpSocket* socket)
:	m_socket(socket)
{
	m_messageTransport = gc_new< MessageTransport >(m_socket);
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

IMessage* Connection::sendMessage(const IMessage& message)
{
	if (!m_messageTransport)
		return 0;

	if (!m_messageTransport->send(&message))
		return 0;

#if !defined(_DEBUG)
	Ref< IMessage > reply = m_messageTransport->receive(10000);
#else
	Ref< IMessage > reply = m_messageTransport->receive(10000000);
#endif
	return reply;
}

	}
}

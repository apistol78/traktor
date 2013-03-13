#include "Core/Thread/Acquire.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteConnection", RemoteConnection, Object)

RemoteConnection::RemoteConnection(net::TcpSocket* socket)
:	m_socket(socket)
{
	m_transport = new net::BidirectionalObjectTransport(m_socket);
}

void RemoteConnection::destroy()
{
	if (m_transport)
		m_transport = 0;

	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
}

void RemoteConnection::setStreamServerAddr(const net::SocketAddressIPv4& streamServerAddr)
{
	m_streamServerAddr = streamServerAddr;
}

const net::SocketAddressIPv4& RemoteConnection::getStreamServerAddr() const
{
	return m_streamServerAddr;
}

Ref< IMessage > RemoteConnection::sendMessage(const IMessage& message)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_transportLock);
	Ref< IMessage > reply;

	if (!m_transport || !m_transport->connected())
		return 0;

	m_transport->flush< IMessage >();

	if (!m_transport->send(&message))
		return 0;

	if (m_transport->recv(60000, reply) <= 0)
		return 0;

	return reply;
}

	}
}

#include "Amalgam/Editor/TargetConnection.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetConnection", TargetConnection, Object)

TargetConnection::TargetConnection(net::TcpSocket* socket)
:	m_socket(socket)
{
}

void TargetConnection::destroy()
{
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
}

bool TargetConnection::update()
{
	if (!m_socket)
		return false;

	if (!m_socket->select(true, false, false, 0))
		return true;

	Ref< TargetPerformance > performance = BinarySerializer(&net::SocketStream(m_socket, true, false)).readObject< TargetPerformance >();
	if (!performance)
		return false;

	m_performance = *performance;
	return true;
}

	}
}

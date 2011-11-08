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

	net::SocketStream ss(m_socket, true, false);

	Ref< TargetPerformance > performance = BinarySerializer(&ss).readObject< TargetPerformance >();
	if (!performance)
		return false;

	Ref< TargetPerformance > deltaPerformance = BinarySerializer(&ss).readObject< TargetPerformance >();
	if (!deltaPerformance)
		return false;

	m_performance = *performance;
	m_deltaPerformance = *deltaPerformance;

	return true;
}

	}
}

#include "Amalgam/Editor/TargetConnection.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
}

void TargetConnection::shutdown()
{
	destroy();
}

bool TargetConnection::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_socket)
		return false;

	if (!m_socket->select(true, false, false, 0))
		return true;

	net::SocketStream ss(m_socket, true, false);

	Ref< TargetPerformance > performance = BinarySerializer(&ss).readObject< TargetPerformance >();
	if (!performance)
		return false;

	m_performance = *performance;
	return true;
}

	}
}

#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Net/SocketStream.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetConnection", TargetConnection, Object)

TargetConnection::TargetConnection(TargetInstance* targetInstance, net::TcpSocket* socket)
:	m_targetInstance(targetInstance)
,	m_socket(socket)
{
}

bool TargetConnection::update()
{
	if (!m_socket->select(true, false, false, 0))
		return true;

	Ref< TargetPerformance > performance = BinarySerializer(&net::SocketStream(m_socket, true, false)).readObject< TargetPerformance >();
	if (!performance)
	{
		if (m_targetInstance->getState() == TsRunning)
			m_targetInstance->setState(TsIdle);
		return false;
	}

	m_targetInstance->setPerformance(*performance);
	return true;
}

	}
}

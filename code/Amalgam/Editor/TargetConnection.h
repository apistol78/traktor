#ifndef traktor_amalgam_TargetConnection_H
#define traktor_amalgam_TargetConnection_H

#include "Net/TcpSocket.h"
#include "Amalgam/Impl/TargetPerformance.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class TargetConnection : public Object
{
	T_RTTI_CLASS;

public:
	TargetConnection(TargetInstance* targetInstance, net::TcpSocket* socket);

	bool update();

	TargetInstance* getTargetInstance() const { return m_targetInstance; }

	net::TcpSocket* getSocket() const { return m_socket; }

private:
	Ref< TargetInstance > m_targetInstance;
	Ref< net::TcpSocket > m_socket;
	TargetPerformance m_performance;
};

	}
}

#endif	// traktor_amalgam_TargetConnection_H

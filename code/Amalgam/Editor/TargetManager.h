#ifndef traktor_amalgam_TargetManager_H
#define traktor_amalgam_TargetManager_H

#include "Core/Thread/Semaphore.h"
#include "Net/SocketSet.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace amalgam
	{

class TargetConnection;
class TargetInstance;

class TargetManager : public Object
{
	T_RTTI_CLASS;

public:
	bool create(uint16_t port, int32_t timeout);

	void destroy();

	bool accept(TargetInstance* targetInstance);

	void update();

private:
	int32_t m_timeout;
	Ref< net::TcpSocket > m_listenSocket;
	RefArray< TargetConnection > m_connections;
	net::SocketSet m_targetSockets;
	Semaphore m_acceptLock;
};

	}
}

#endif	// traktor_amalgam_TargetManager_H

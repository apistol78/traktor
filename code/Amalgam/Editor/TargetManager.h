#ifndef traktor_amalgam_TargetManager_H
#define traktor_amalgam_TargetManager_H

#include "Core/RefArray.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace amalgam
	{

class Target;
class TargetConnection;
class TargetInstance;

class TargetManager : public Object
{
	T_RTTI_CLASS;

public:
	bool create(uint16_t port);

	void destroy();

	TargetInstance* createInstance(const std::wstring& name, const Target* target);

	void update();

private:
	Ref< net::TcpSocket > m_listenSocket;
	RefArray< TargetInstance > m_instances;
};

	}
}

#endif	// traktor_amalgam_TargetManager_H

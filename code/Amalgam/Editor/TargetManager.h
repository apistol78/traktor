#ifndef traktor_amalgam_TargetManager_H
#define traktor_amalgam_TargetManager_H

#include "Core/RefArray.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;
class TargetScriptDebuggerSessions;

class TargetManager : public Object
{
	T_RTTI_CLASS;

public:
	TargetManager(ILogTarget* targetLog, TargetScriptDebuggerSessions* targetDebuggerSessions);

	bool create(uint16_t port);

	void destroy();

	void addInstance(TargetInstance* targetInstance);

	void removeInstance(TargetInstance* targetInstance);

	bool update();

private:
	Ref< ILogTarget > m_targetLog;
	Ref< TargetScriptDebuggerSessions > m_targetDebuggerSessions;
	Ref< net::TcpSocket > m_listenSocket;
	RefArray< TargetInstance > m_instances;
};

	}
}

#endif	// traktor_amalgam_TargetManager_H

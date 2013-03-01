#ifndef traktor_amalgam_ScriptServer_H
#define traktor_amalgam_ScriptServer_H

#include "Amalgam/IScriptServer.h"
#include "Core/Thread/Thread.h"
#include "Script/IScriptDebugger.h"

namespace traktor
{

class PropertyGroup;

	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace amalgam
	{

class IEnvironment;

class ScriptServer
:	public IScriptServer
,	public script::IScriptDebugger::IListener
{
	T_RTTI_CLASS;

public:
	ScriptServer();

	bool create(const PropertyGroup* settings, bool debugger, net::BidirectionalObjectTransport* transport);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update();

	virtual script::IScriptManager* getScriptManager();

private:
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptDebugger > m_scriptDebugger;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Thread* m_scriptDebuggerThread;

	void threadDebugger();

	virtual void breakpointReached(script::IScriptDebugger* scriptDebugger, const script::CallStack& callStack);
};

	}
}

#endif	// traktor_amalgam_ScriptServer_H

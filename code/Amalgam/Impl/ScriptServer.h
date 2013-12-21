#ifndef traktor_amalgam_ScriptServer_H
#define traktor_amalgam_ScriptServer_H

#include "Amalgam/IScriptServer.h"
#include "Core/Thread/Thread.h"
#include "Script/IScriptDebugger.h"
#include "Script/IScriptProfiler.h"

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
,	public script::IScriptProfiler::IListener
{
	T_RTTI_CLASS;

public:
	ScriptServer();

	bool create(const PropertyGroup* settings, bool debugger, bool profiler, net::BidirectionalObjectTransport* transport);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void cleanup(bool full);

	virtual script::IScriptManager* getScriptManager();

private:
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptDebugger > m_scriptDebugger;
	Ref< script::IScriptProfiler > m_scriptProfiler;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Thread* m_scriptDebuggerThread;

	void threadDebugger();

	virtual void breakpointReached(script::IScriptDebugger* scriptDebugger, const script::CallStack& callStack);

	virtual void callMeasured(const std::wstring& function, double timeStamp, double inclusiveDuration, double exclusiveDuration);
};

	}
}

#endif	// traktor_amalgam_ScriptServer_H

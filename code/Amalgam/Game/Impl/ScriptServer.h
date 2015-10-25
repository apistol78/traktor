#ifndef traktor_amalgam_ScriptServer_H
#define traktor_amalgam_ScriptServer_H

#include <map>
#include "Amalgam/Game/IScriptServer.h"
#include "Core/Thread/Semaphore.h"
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

	bool create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, bool debugger, bool profiler, net::BidirectionalObjectTransport* transport);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void cleanup(bool full);

	virtual script::IScriptManager* getScriptManager() T_OVERRIDE T_FINAL;

private:
	struct CallSample
	{
		uint32_t callCount;
		double inclusiveDuration;
		double exclusiveDuration;

		CallSample()
		:	callCount(0)
		,	inclusiveDuration(0.0)
		,	exclusiveDuration(0.0)
		{
		}
	};

	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptDebugger > m_scriptDebugger;
	Ref< script::IScriptProfiler > m_scriptProfiler;
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::map< std::pair< Guid, std::wstring >, CallSample > m_callSamples[3];
	int32_t m_callSamplesIndex;
	Thread* m_scriptDebuggerThread;

	void threadDebugger();

	virtual void breakpointReached(script::IScriptDebugger* scriptDebugger);

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);
};

	}
}

#endif	// traktor_amalgam_ScriptServer_H

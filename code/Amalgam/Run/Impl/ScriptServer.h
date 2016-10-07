#ifndef traktor_amalgam_ScriptServer_H
#define traktor_amalgam_ScriptServer_H

#include <map>
#include "Amalgam/Run/IScriptServer.h"
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

	namespace script
	{

class IScriptContext;

	}

	namespace amalgam
	{

class IEnvironment;

/*! \brief
 * \ingroup Amalgam
 */
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

	bool execute(IEnvironment* environment);

	bool update();

	virtual script::IScriptManager* getScriptManager();

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
	Ref< script::IScriptContext > m_scriptContext;
	Ref< script::IScriptDebugger > m_scriptDebugger;
	Ref< script::IScriptProfiler > m_scriptProfiler;
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::map< std::pair< Guid, std::wstring >, CallSample > m_callSamples[3];
	int32_t m_callSamplesIndex;
	Thread* m_executionThread;
	Thread* m_scriptDebuggerThread;

	void threadExecution();

	void threadDebugger();

	virtual void debugeeStateChange(script::IScriptDebugger* scriptDebugger) T_OVERRIDE T_FINAL;

	virtual void callEnter(const Guid& scriptId, const std::wstring& function) T_OVERRIDE T_FINAL;

	virtual void callLeave(const Guid& scriptId, const std::wstring& function) T_OVERRIDE T_FINAL;

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_ScriptServer_H

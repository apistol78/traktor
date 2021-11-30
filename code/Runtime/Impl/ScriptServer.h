#pragma once

#include <map>
#include "Runtime/IScriptServer.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Script/IScriptDebugger.h"
#include "Script/IScriptProfiler.h"

namespace traktor
{

class PropertyGroup;

	namespace input
	{

class InputSystem;

	}

	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace runtime
	{

class IEnvironment;

/*! \brief
 * \ingroup Runtime
 */
class ScriptServer
:	public IScriptServer
,	public script::IScriptDebugger::IListener
,	public script::IScriptProfiler::IListener
{
	T_RTTI_CLASS;

public:
	ScriptServer();

	bool create(
		const PropertyGroup* defaultSettings,
		const PropertyGroup* settings,
		bool debugger,
		bool profiler,
		input::InputSystem* inputSystem,
		net::BidirectionalObjectTransport* transport
	);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void cleanup(bool full);

	virtual script::IScriptManager* getScriptManager() override final;

	virtual script::IScriptContext* getScriptContext() override final;

private:
	struct CallSample
	{
		uint32_t callCount = 0;
		double inclusiveDuration = 0.0;
		double exclusiveDuration = 0.0;
	};

	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptContext > m_scriptContext;
	Ref< script::IScriptDebugger > m_scriptDebugger;
	Ref< script::IScriptProfiler > m_scriptProfiler;
	Ref< input::InputSystem > m_inputSystem;
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::map< std::pair< Guid, std::wstring >, CallSample > m_callSamples[3];
	std::atomic< int32_t > m_callSamplesIndex;
	Thread* m_scriptDebuggerThread = nullptr;

	void threadDebugger();

	virtual void debugeeStateChange(script::IScriptDebugger* scriptDebugger) override final;

	virtual void callEnter(const Guid& scriptId, const std::wstring& function) override final;

	virtual void callLeave(const Guid& scriptId, const std::wstring& function) override final;

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) override final;
};

	}
}


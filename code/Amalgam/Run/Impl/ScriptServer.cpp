#include "Amalgam/ScriptDebuggerBreakpoint.h"
#include "Amalgam/ScriptDebuggerControl.h"
#include "Amalgam/ScriptDebuggerHalted.h"
#include "Amalgam/ScriptProfilerCallMeasured.h"
#include "Amalgam/Run/IEnvironment.h"
#include "Amalgam/Run/Impl/LibraryHelper.h"
#include "Amalgam/Run/Impl/ScriptServer.h"
#include "Core/Class/CastAny.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ScriptServer", ScriptServer, IScriptServer)

ScriptServer::ScriptServer()
:	m_callSamplesIndex(0)
,	m_executionThread(0)
,	m_scriptDebuggerThread(0)
{
}

bool ScriptServer::create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, bool debugger, bool profiler, net::BidirectionalObjectTransport* transport)
{
	std::wstring scriptType = defaultSettings->getProperty< PropertyString >(L"Script.Type");

	m_scriptManager = loadAndInstantiate< script::IScriptManager >(scriptType);
	if (!m_scriptManager)
		return false;

	// Create and attach debugger.
	if (debugger)
	{
		m_scriptDebugger = m_scriptManager->createDebugger();
		if (m_scriptDebugger)
		{
			m_scriptDebugger->addListener(this);
		}
		else
		{
			log::warning << L"Unable to create script debugger" << Endl;
			debugger = false;
		}
	}

	// Create and attach profiler.
	if (profiler)
	{
		m_scriptProfiler = m_scriptManager->createProfiler();
		if (m_scriptProfiler)
		{
			m_scriptProfiler->addListener(this);
		}
		else
		{
			log::warning << L"Unable to create script profiler" << Endl;
			profiler = false;
		}
	}

	// Create debugger/profiler thread if any of those is attached.
	if (debugger || profiler)
	{
		m_transport = transport;

		m_scriptDebuggerThread = ThreadManager::getInstance().create(makeFunctor(this, &ScriptServer::threadDebugger), L"Script debugger/profiler thread");
		if (!m_scriptDebuggerThread)
			return false;

		m_scriptDebuggerThread->start();
	}

	return true;
}

void ScriptServer::destroy()
{
	if (m_executionThread)
	{
		m_executionThread->stop();
		ThreadManager::getInstance().destroy(m_executionThread);
		m_executionThread = 0;
	}

	if (m_scriptDebuggerThread)
	{
		m_scriptDebuggerThread->stop();
		ThreadManager::getInstance().destroy(m_scriptDebuggerThread);
		m_scriptDebuggerThread = 0;
	}

	if (m_scriptDebugger)
	{
		m_scriptDebugger->removeListener(this);
		m_scriptDebugger = 0;
	}

	if (m_scriptProfiler)
	{
		m_scriptProfiler->removeListener(this);
		m_scriptProfiler = 0;
	}

	safeDestroy(m_scriptContext);
	safeDestroy(m_scriptManager);
}

bool ScriptServer::execute(IEnvironment* environment)
{
	// Create script context.
	Guid startupGuid(environment->getSettings()->getProperty< PropertyString >(L"Amalgam.Startup"));

	Ref< script::IScriptResource > scriptResource = environment->getDatabase()->getObjectReadOnly< script::IScriptResource >(startupGuid);
	if (!scriptResource)
	{
		log::error << L"Unable to load script resource" << Endl;
		return false;
	}

	m_scriptContext = m_scriptManager->createContext(scriptResource, 0);
	if (!m_scriptContext)
	{
		log::error << L"Unable to create script execution context" << Endl;
		return false;
	}

	m_scriptContext->setGlobal("environment", Any::fromObject(environment));

	// Create execution thread.
	m_executionThread = ThreadManager::getInstance().create(makeFunctor(this, &ScriptServer::threadExecution), L"Script execution thread");
	if (!m_executionThread)
		return false;

	m_executionThread->start();
	return true;
}

bool ScriptServer::update()
{
	if (m_executionThread)
	{
		if (!m_executionThread->wait(100))
			return true;
	}
	return false;
}

script::IScriptManager* ScriptServer::getScriptManager()
{
	return m_scriptManager;
}

void ScriptServer::threadExecution()
{
	m_scriptContext->executeFunction("main");
}

void ScriptServer::threadDebugger()
{
	while (!m_scriptDebuggerThread->stopped() && m_transport->connected())
	{
		if (!m_transport->wait(500))
		{
			int32_t index = (m_callSamplesIndex + 1) % 3;

			std::map< std::pair< Guid, std::wstring >, CallSample >& samples = m_callSamples[index];
			for (std::map< std::pair< Guid, std::wstring >, CallSample >::const_iterator i = samples.begin(); i != samples.end(); ++i)
			{
				ScriptProfilerCallMeasured measured(i->first.first, i->first.second, i->second.callCount, i->second.inclusiveDuration, i->second.exclusiveDuration);
				m_transport->send(&measured);
			}
			samples.clear();

			Atomic::exchange(m_callSamplesIndex, index);
			continue;
		}

		if (m_scriptDebugger)
		{
			Ref< ScriptDebuggerBreakpoint > breakpoint;
			if (m_transport->recv< ScriptDebuggerBreakpoint >(0, breakpoint) == net::BidirectionalObjectTransport::RtSuccess)
			{
				T_ASSERT (breakpoint);
				if (breakpoint->shouldAdd())
					m_scriptDebugger->setBreakpoint(breakpoint->getScriptId(), breakpoint->getLineNumber());
				else
					m_scriptDebugger->removeBreakpoint(breakpoint->getScriptId(), breakpoint->getLineNumber());
			}

			Ref< ScriptDebuggerControl > control;
			if (m_transport->recv< ScriptDebuggerControl >(0, control) == net::BidirectionalObjectTransport::RtSuccess)
			{
				T_ASSERT (control);
				switch (control->getAction())
				{
				case ScriptDebuggerControl::AcBreak:
					m_scriptDebugger->actionBreak();
					break;

				case ScriptDebuggerControl::AcContinue:
					m_scriptDebugger->actionContinue();
					break;

				case ScriptDebuggerControl::AcStepInto:
					m_scriptDebugger->actionStepInto();
					break;

				case ScriptDebuggerControl::AcStepOver:
					m_scriptDebugger->actionStepOver();
					break;
				}
			}
		}
	}

	if (m_scriptDebugger)
		m_scriptDebugger->actionContinue();
}

void ScriptServer::breakpointReached(script::IScriptDebugger* scriptDebugger, const script::CallStack& callStack)
{
	ScriptDebuggerHalted halted(callStack);
	m_transport->send(&halted);
}

void ScriptServer::callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
{
	CallSample& sample = m_callSamples[m_callSamplesIndex][std::make_pair(scriptId, function)];
	sample.callCount += callCount;
	sample.inclusiveDuration += inclusiveDuration;
	sample.exclusiveDuration += exclusiveDuration;
}

	}
}

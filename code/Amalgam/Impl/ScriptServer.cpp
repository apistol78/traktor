#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Amalgam/Impl/ScriptServer.h"
#include "Amalgam/Impl/ScriptDebuggerBreakpoint.h"
#include "Amalgam/Impl/ScriptDebuggerControl.h"
#include "Amalgam/Impl/ScriptDebuggerHalted.h"
#include "Amalgam/Impl/ScriptProfilerCallMeasured.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptContextFactory.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ScriptServer", ScriptServer, IScriptServer)

ScriptServer::ScriptServer()
:	m_scriptDebuggerThread(0)
{
}

bool ScriptServer::create(const PropertyGroup* settings, bool debugger, bool profiler, net::BidirectionalObjectTransport* transport)
{
	std::wstring scriptType = settings->getProperty< PropertyString >(L"Script.Type");

	m_scriptManager = loadAndInstantiate< script::IScriptManager >(scriptType);
	if (!m_scriptManager)
		return false;

	if (debugger)
	{
		m_scriptDebugger = m_scriptManager->createDebugger();
		if (m_scriptDebugger)
		{
			m_scriptDebuggerThread = ThreadManager::getInstance().create(makeFunctor(this, &ScriptServer::threadDebugger), L"Script debugger thread");
			if (!m_scriptDebuggerThread)
				return false;

			m_transport = transport;
			m_scriptDebugger->addListener(this);

			m_scriptDebuggerThread->start();
		}
		else
			log::warning << L"Unable to create script debugger" << Endl;
	}

	if (profiler)
	{
		m_scriptProfiler = m_scriptManager->createProfiler();
		if (m_scriptProfiler)
		{
			m_scriptProfiler->addListener(this);
		}
		else
			log::warning << L"Unable to create script profiler" << Endl;
	}

	return true;
}

void ScriptServer::destroy()
{
	if (m_scriptDebugger)
	{
		m_scriptDebugger->removeListener(this);
		m_scriptDebugger = 0;
	}

	if (m_scriptDebuggerThread)
	{
		m_scriptDebuggerThread->stop();
		ThreadManager::getInstance().destroy(m_scriptDebuggerThread);
		m_scriptDebuggerThread = 0;
	}

	if (m_scriptProfiler)
	{
		m_scriptProfiler->removeListener(this);
		m_scriptProfiler = 0;
	}

	safeDestroy(m_scriptManager);
}

void ScriptServer::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new script::ScriptContextFactory(database, m_scriptManager));
}

int32_t ScriptServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void ScriptServer::cleanup(bool full)
{
	T_ASSERT (m_scriptManager);
	m_scriptManager->collectGarbage(full);
}

script::IScriptManager* ScriptServer::getScriptManager()
{
	return m_scriptManager;
}

void ScriptServer::threadDebugger()
{
	while (!m_scriptDebuggerThread->stopped() && m_transport->connected())
	{
		if (!m_transport->wait(100))
			continue;

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
	if (m_scriptDebugger)
		m_scriptDebugger->actionContinue();
}

void ScriptServer::breakpointReached(script::IScriptDebugger* scriptDebugger, const script::CallStack& callStack)
{
	ScriptDebuggerHalted halted(callStack);
	m_transport->send(&halted);
}

void ScriptServer::callMeasured(const std::wstring& function, double timeStamp, double inclusiveDuration, double exclusiveDuration)
{
	ScriptProfilerCallMeasured measured(function, timeStamp, inclusiveDuration, exclusiveDuration);
	m_transport->send(&measured);
}

	}
}

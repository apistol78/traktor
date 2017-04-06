#include "Amalgam/ScriptDebuggerBreakpoint.h"
#include "Amalgam/ScriptDebuggerControl.h"
#include "Amalgam/ScriptDebuggerLocals.h"
#include "Amalgam/ScriptDebuggerStackFrame.h"
#include "Amalgam/ScriptDebuggerStateChange.h"
#include "Amalgam/ScriptDebuggerStatus.h"
#include "Amalgam/ScriptProfilerCallMeasured.h"
#include "Amalgam/Run/IEnvironment.h"
#include "Amalgam/Run/Impl/ScriptServer.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
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
#include "Script/ScriptResource.h"
#include "Script/StackFrame.h"

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

	m_scriptManager = dynamic_type_cast< script::IScriptManager* >(TypeInfo::createInstance(scriptType));
	if (!m_scriptManager)
		return false;

	// Register all runtime classes, first collect all classes
	// and then register them in class dependency order.
	OrderedClassRegistrar registrar;
	std::set< const TypeInfo* > runtimeClassFactoryTypes;
	type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
	for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}
	registrar.registerClassesInOrder(m_scriptManager);

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

	Ref< script::ScriptResource > scriptResource = environment->getDatabase()->getObjectReadOnly< script::ScriptResource >(startupGuid);
	if (!scriptResource)
	{
		log::error << L"Unable to load script resource" << Endl;
		return false;
	}

	m_scriptContext = m_scriptManager->createContext(false);
	if (!m_scriptContext)
	{
		log::error << L"Unable to create script execution context" << Endl;
		return false;
	}

	m_scriptContext->load(scriptResource->getBlob());
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
		if (m_scriptDebugger)
		{
			Ref< ISerializable > object;
			if (m_transport->recv(makeTypeInfoSet< ScriptDebuggerBreakpoint, ScriptDebuggerControl >(), 500, object) == net::BidirectionalObjectTransport::RtSuccess)
			{
				if (ScriptDebuggerBreakpoint* breakpoint = dynamic_type_cast< ScriptDebuggerBreakpoint* >(object))
				{
					if (breakpoint->shouldAdd())
					{
						m_scriptDebugger->setBreakpoint(breakpoint->getScriptId(), breakpoint->getLineNumber());
						log::debug << L"Breakpoint " << breakpoint->getScriptId().format() << L":" << breakpoint->getLineNumber() << L" added." << Endl;
					}
					else
					{
						m_scriptDebugger->removeBreakpoint(breakpoint->getScriptId(), breakpoint->getLineNumber());
						log::debug << L"Breakpoint " << breakpoint->getScriptId().format() << L":" << breakpoint->getLineNumber() << L" removed." << Endl;
					}
				}
				else if (ScriptDebuggerControl* control = dynamic_type_cast< ScriptDebuggerControl* >(object))
				{
					switch (control->getAction())
					{
					case ScriptDebuggerControl::AcStatus:
					{
						ScriptDebuggerStatus status(m_scriptDebugger->isRunning());
						m_transport->send(&status);
					}
					break;

					case ScriptDebuggerControl::AcBreak:
					{
						m_scriptDebugger->actionBreak();
					}
					break;

					case ScriptDebuggerControl::AcContinue:
					{
						m_scriptDebugger->actionContinue();
					}
					break;

					case ScriptDebuggerControl::AcStepInto:
					{
						m_scriptDebugger->actionStepInto();
					}
					break;

					case ScriptDebuggerControl::AcStepOver:
					{
						m_scriptDebugger->actionStepOver();
					}
					break;

					case ScriptDebuggerControl::AcCaptureStack:
					{
						Ref< script::StackFrame > sf;
						if (!m_scriptDebugger->captureStackFrame(control->getParam(), sf))
							sf = 0;
						ScriptDebuggerStackFrame capturedFrame(sf);
						m_transport->send(&capturedFrame);
					}
					break;

					case ScriptDebuggerControl::AcCaptureLocals:
					{
						RefArray< script::Variable > l;
						if (m_scriptDebugger->captureLocals(control->getParam(), l))
						{
							ScriptDebuggerLocals capturedLocals(l);
							m_transport->send(&capturedLocals);
						}
					}
					break;

					case ScriptDebuggerControl::AcCaptureObject:
					{
						RefArray< script::Variable > l;
						if (m_scriptDebugger->captureObject(control->getParam(), l))
						{
							ScriptDebuggerLocals capturedLocals(l);
							m_transport->send(&capturedLocals);
						}
					}
					break;
					}
				}
			}
		}
		else
			ThreadManager::getInstance().getCurrentThread()->sleep(500);

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
		}
	}

	if (m_scriptDebugger)
		m_scriptDebugger->actionContinue();
}

void ScriptServer::debugeeStateChange(script::IScriptDebugger* scriptDebugger)
{
	ScriptDebuggerStateChange stateChange;
	m_transport->send(&stateChange);
}

void ScriptServer::callEnter(const Guid& scriptId, const std::wstring& function)
{
}

void ScriptServer::callLeave(const Guid& scriptId, const std::wstring& function)
{
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

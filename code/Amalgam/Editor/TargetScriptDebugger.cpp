#include "Amalgam/ScriptDebuggerBreakpoint.h"
#include "Amalgam/ScriptDebuggerControl.h"
#include "Amalgam/ScriptDebuggerLocals.h"
#include "Amalgam/ScriptDebuggerStateChange.h"
#include "Amalgam/ScriptDebuggerStackFrame.h"
#include "Amalgam/ScriptDebuggerStatus.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Core/Log/Log.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const int32_t c_timeout = 30000;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetScriptDebugger", TargetScriptDebugger, script::IScriptDebugger)

TargetScriptDebugger::TargetScriptDebugger(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
{
}

void TargetScriptDebugger::update()
{
	Ref< ScriptDebuggerStateChange > debugger;
	while (m_transport->recv< ScriptDebuggerStateChange >(0, debugger) == net::BidirectionalObjectTransport::RtSuccess)
	{
		for (std::list< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->debugeeStateChange(this);
	}
}

bool TargetScriptDebugger::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	ScriptDebuggerBreakpoint bp(true, scriptId, lineNumber);
	if (!m_transport->send(&bp))
	{
		log::error << L"Target script debugger error; Unable to send while setting breakpoint." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while setting breakpoint." << Endl;
		return false;
	}

	return true;
}

bool TargetScriptDebugger::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	ScriptDebuggerBreakpoint bp(false, scriptId, lineNumber);
	if (!m_transport->send(&bp))
	{
		log::error << L"Target script debugger error; Unable to send while setting breakpoint." << Endl;
		return false;
	}


	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while removing breakpoint." << Endl;
		return false;
	}

	return true;
}

bool TargetScriptDebugger::captureStackFrame(uint32_t depth, Ref< script::StackFrame >& outStackFrame)
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureStack, depth);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting stack frame." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStackFrame > sf;
	if (m_transport->recv< ScriptDebuggerStackFrame >(c_timeout, sf) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No response while requesting stack frame." << Endl;
		return false;
	}

	outStackFrame = sf->getFrame();
	return outStackFrame != 0;
}

bool TargetScriptDebugger::captureLocals(uint32_t depth, RefArray< script::Variable >& outLocals)
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureLocals, depth);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting locals." << Endl;
		return false;
	}

	Ref< ScriptDebuggerLocals > l;
	if (m_transport->recv< ScriptDebuggerLocals >(c_timeout, l) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No response while requesting locals." << Endl;
		return false;
	}

	outLocals = l->getLocals();
	return true;
}

bool TargetScriptDebugger::captureObject(uint32_t object, RefArray< script::Variable >& outMembers)
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureObject, object);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting object capture." << Endl;
		return false;
	}

	Ref< ScriptDebuggerLocals > l;
	if (m_transport->recv< ScriptDebuggerLocals >(c_timeout, l) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No response while requesting object capture." << Endl;
		return false;
	}

	outMembers = l->getLocals();
	return true;
}

bool TargetScriptDebugger::isRunning() const
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStatus);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while checking running state." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while checking running state." << Endl;
		return false;
	}

	return st->isRunning();
}

bool TargetScriptDebugger::actionBreak()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcBreak);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while break target." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while break target." << Endl;
		return false;
	}

	return true;
}

bool TargetScriptDebugger::actionContinue()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcContinue);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while continue target." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while continue target." << Endl;
		return false;
	}

	return true;
}

bool TargetScriptDebugger::actionStepInto()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepInto);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while step-into target." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while step-into target." << Endl;
		return false;
	}

	return true;
}

bool TargetScriptDebugger::actionStepOver()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepOver);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while step-over target." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(c_timeout, st) != net::BidirectionalObjectTransport::RtSuccess)
	{
		log::error << L"Target script debugger error; No status response received while step-over target." << Endl;
		return false;
	}

	return true;
}

void TargetScriptDebugger::addListener(IListener* listener)
{
	T_ASSERT (listener);
	m_listeners.push_back(listener);
	listener->debugeeStateChange(this);
}

void TargetScriptDebugger::removeListener(IListener* listener)
{
	T_ASSERT (listener);
	m_listeners.remove(listener);
}

	}
}

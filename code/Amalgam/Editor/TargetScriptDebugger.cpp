#include "Amalgam/ScriptDebuggerBreakpoint.h"
#include "Amalgam/ScriptDebuggerControl.h"
#include "Amalgam/ScriptDebuggerLocals.h"
#include "Amalgam/ScriptDebuggerStateChange.h"
#include "Amalgam/ScriptDebuggerStackFrame.h"
#include "Amalgam/ScriptDebuggerStatus.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace amalgam
	{

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
		return false;

	Ref< ScriptDebuggerStatus > st;
	return m_transport->recv< ScriptDebuggerStatus >(1000, st) == net::BidirectionalObjectTransport::RtSuccess;
}

bool TargetScriptDebugger::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	ScriptDebuggerBreakpoint bp(false, scriptId, lineNumber);
	if (!m_transport->send(&bp))
		return false;

	Ref< ScriptDebuggerStatus > st;
	return m_transport->recv< ScriptDebuggerStatus >(1000, st) == net::BidirectionalObjectTransport::RtSuccess;
}

bool TargetScriptDebugger::captureStackFrame(uint32_t depth, Ref< script::StackFrame >& outStackFrame)
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureStack, depth);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerStackFrame > sf;
	if (m_transport->recv< ScriptDebuggerStackFrame >(1000, sf) != net::BidirectionalObjectTransport::RtSuccess)
		return false;

	outStackFrame = sf->getFrame();
	return outStackFrame != 0;
}

bool TargetScriptDebugger::captureLocals(uint32_t depth, RefArray< script::Local >& outLocals)
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureLocals, depth);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerLocals > l;
	if (m_transport->recv< ScriptDebuggerLocals >(1000, l) != net::BidirectionalObjectTransport::RtSuccess)
		return false;

	outLocals = l->getLocals();
	return true;
}

bool TargetScriptDebugger::isRunning() const
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStatus);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerStatus > st;
	if (m_transport->recv< ScriptDebuggerStatus >(1000, st) != net::BidirectionalObjectTransport::RtSuccess)
		return false;

	return st->isRunning();
}

bool TargetScriptDebugger::actionBreak()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcBreak);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerStatus > st;
	return m_transport->recv< ScriptDebuggerStatus >(1000, st) == net::BidirectionalObjectTransport::RtSuccess;
}

bool TargetScriptDebugger::actionContinue()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcContinue);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerStatus > st;
	return m_transport->recv< ScriptDebuggerStatus >(1000, st) == net::BidirectionalObjectTransport::RtSuccess;
}

bool TargetScriptDebugger::actionStepInto()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepInto);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerStatus > st;
	return m_transport->recv< ScriptDebuggerStatus >(1000, st) == net::BidirectionalObjectTransport::RtSuccess;
}

bool TargetScriptDebugger::actionStepOver()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepOver);
	if (!m_transport->send(&ctrl))
		return false;

	Ref< ScriptDebuggerStatus > st;
	return m_transport->recv< ScriptDebuggerStatus >(1000, st) == net::BidirectionalObjectTransport::RtSuccess;
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

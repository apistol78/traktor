#include "Amalgam/ScriptDebuggerBreakpoint.h"
#include "Amalgam/ScriptDebuggerControl.h"
#include "Amalgam/ScriptDebuggerStackFrame.h"
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

bool TargetScriptDebugger::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	ScriptDebuggerBreakpoint bp(true, scriptId, lineNumber);
	return m_transport->send(&bp);
}

bool TargetScriptDebugger::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	ScriptDebuggerBreakpoint bp(false, scriptId, lineNumber);
	return m_transport->send(&bp);
}

Ref< script::StackFrame > TargetScriptDebugger::captureStackFrame(uint32_t depth)
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCapture, depth);
	if (!m_transport->send(&ctrl))
		return 0;

	Ref< ScriptDebuggerStackFrame > sf;
	if (m_transport->recv< ScriptDebuggerStackFrame >(1000, sf) != net::BidirectionalObjectTransport::RtSuccess)
		return 0;

	return sf->getFrame();
}

bool TargetScriptDebugger::isRunning() const
{
	return true;
}

bool TargetScriptDebugger::actionBreak()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcBreak);
	return m_transport->send(&ctrl);
}

bool TargetScriptDebugger::actionContinue()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcContinue);
	return m_transport->send(&ctrl);
}

bool TargetScriptDebugger::actionStepInto()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepInto);
	return m_transport->send(&ctrl);
}

bool TargetScriptDebugger::actionStepOver()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepOver);
	return m_transport->send(&ctrl);
}

void TargetScriptDebugger::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

void TargetScriptDebugger::removeListener(IListener* listener)
{
	m_listeners.remove(listener);
}

void TargetScriptDebugger::notifyListeners()
{
	for (std::list< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->breakpointReached(this);
}

	}
}

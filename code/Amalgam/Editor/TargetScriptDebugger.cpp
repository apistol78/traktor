#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Amalgam/Impl/ScriptDebuggerBreakpoint.h"
#include "Amalgam/Impl/ScriptDebuggerControl.h"
#include "Core/Guid.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Script/CallStack.h"

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
	m_transport->send(&bp);
	return true;
}

bool TargetScriptDebugger::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	ScriptDebuggerBreakpoint bp(false, scriptId, lineNumber);
	m_transport->send(&bp);
	return true;
}

bool TargetScriptDebugger::isRunning() const
{
	return true;
}

bool TargetScriptDebugger::actionBreak()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcBreak);
	m_transport->send(&ctrl);
	m_currentCallStack = 0;
	return true;
}

bool TargetScriptDebugger::actionContinue()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcContinue);
	m_transport->send(&ctrl);
	m_currentCallStack = 0;
	return true;
}

bool TargetScriptDebugger::actionStepInto()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepInto);
	m_transport->send(&ctrl);
	m_currentCallStack = 0;
	return true;
}

bool TargetScriptDebugger::actionStepOver()
{
	ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepOver);
	m_transport->send(&ctrl);
	m_currentCallStack = 0;
	return true;
}

void TargetScriptDebugger::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
	if (m_currentCallStack)
		listener->breakpointReached(this, *m_currentCallStack);
}

void TargetScriptDebugger::removeListener(IListener* listener)
{
	m_listeners.remove(listener);
}

void TargetScriptDebugger::notifyListeners(const script::CallStack& callStack)
{
	m_currentCallStack = new script::CallStack(callStack);
	for (std::list< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->breakpointReached(this, *m_currentCallStack);
}

	}
}

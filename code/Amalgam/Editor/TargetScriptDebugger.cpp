#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Amalgam/Impl/ScriptDebuggerBreakpoint.h"
#include "Amalgam/Impl/ScriptDebuggerControl.h"
#include "Core/Guid.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetScriptDebugger", TargetScriptDebugger, script::IScriptDebugger)

bool TargetScriptDebugger::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	for (std::list< TargetConnection* >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->getTransport()->send(&ScriptDebuggerBreakpoint(true, scriptId, lineNumber));

	m_breakpoints.push_back(std::make_pair(scriptId, lineNumber));
	return true;
}

bool TargetScriptDebugger::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	for (std::list< TargetConnection* >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->getTransport()->send(&ScriptDebuggerBreakpoint(false, scriptId, lineNumber));

	m_breakpoints.remove(std::make_pair(scriptId, lineNumber));
	return true;
}

bool TargetScriptDebugger::isRunning()
{
	return true;
}

bool TargetScriptDebugger::actionBreak()
{
	for (std::list< TargetConnection* >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->getTransport()->send(&ScriptDebuggerControl(ScriptDebuggerControl::AcBreak));
	return true;
}

bool TargetScriptDebugger::actionContinue()
{
	for (std::list< TargetConnection* >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->getTransport()->send(&ScriptDebuggerControl(ScriptDebuggerControl::AcContinue));
	return true;
}

bool TargetScriptDebugger::actionStepInto()
{
	for (std::list< TargetConnection* >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->getTransport()->send(&ScriptDebuggerControl(ScriptDebuggerControl::AcStepInto));
	return true;
}

bool TargetScriptDebugger::actionStepOver()
{
	for (std::list< TargetConnection* >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->getTransport()->send(&ScriptDebuggerControl(ScriptDebuggerControl::AcStepOver));
	return true;
}

void TargetScriptDebugger::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

void TargetScriptDebugger::removeListener(IListener* listener)
{
	m_listeners.remove(listener);
}

void TargetScriptDebugger::addConnection(TargetConnection* connection)
{
	for (std::list< std::pair< Guid, int32_t > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
		connection->getTransport()->send(&ScriptDebuggerBreakpoint(true, i->first, i->second));

	m_connections.push_back(connection);
}

void TargetScriptDebugger::removeConnection(TargetConnection* connection)
{
	m_connections.remove(connection);
}

void TargetScriptDebugger::notifyListeners(const script::CallStack& callStack)
{
	for (std::list< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->breakpointReached(this, callStack);
}

	}
}

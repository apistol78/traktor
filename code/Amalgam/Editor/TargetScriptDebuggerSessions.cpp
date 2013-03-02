#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Amalgam/Editor/TargetScriptDebuggerSessions.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetScriptDebuggerSessions", TargetScriptDebuggerSessions, script::IScriptDebuggerSessions)

void TargetScriptDebuggerSessions::beginSession(TargetScriptDebugger* scriptDebugger)
{
	T_ASSERT (scriptDebugger);

	m_scriptDebuggers.push_back(scriptDebugger);

	for (std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
	{
		for (std::set< Guid >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			scriptDebugger->setBreakpoint(*j, i->first);
	}

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyDebuggerBeginSession(scriptDebugger);
}

void TargetScriptDebuggerSessions::endSession(TargetScriptDebugger* scriptDebugger)
{
	T_ASSERT (scriptDebugger);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyDebuggerEndSession(scriptDebugger);

	for (std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
	{
		for (std::set< Guid >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			scriptDebugger->removeBreakpoint(*j, i->first);
	}

	m_scriptDebuggers.remove(scriptDebugger);
}

bool TargetScriptDebuggerSessions::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	m_breakpoints[lineNumber].insert(scriptId);

	for (RefArray< TargetScriptDebugger >::iterator i = m_scriptDebuggers.begin(); i != m_scriptDebuggers.end(); ++i)
		(*i)->setBreakpoint(scriptId, lineNumber);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyDebuggerSetBreakpoint(scriptId, lineNumber);

	return true;
}

bool TargetScriptDebuggerSessions::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	m_breakpoints[lineNumber].erase(scriptId);

	for (RefArray< TargetScriptDebugger >::iterator i = m_scriptDebuggers.begin(); i != m_scriptDebuggers.end(); ++i)
		(*i)->removeBreakpoint(scriptId, lineNumber);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyDebuggerRemoveBreakpoint(scriptId, lineNumber);

	return true;
}

bool TargetScriptDebuggerSessions::haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const
{
	std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.find(lineNumber);
	if (i != m_breakpoints.end())
		return i->second.find(scriptId) != i->second.end();
	else
		return false;
}

void TargetScriptDebuggerSessions::addListener(IListener* listener)
{
	T_ASSERT (listener);
	
	for (RefArray< TargetScriptDebugger >::iterator i = m_scriptDebuggers.begin(); i != m_scriptDebuggers.end(); ++i)
		listener->notifyDebuggerBeginSession(*i);

	for (std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
	{
		for (std::set< Guid >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			listener->notifyDebuggerSetBreakpoint(*j, i->first);
	}

	m_listeners.push_back(listener);
}

void TargetScriptDebuggerSessions::removeListener(IListener* listener)
{
	T_ASSERT (listener);
	for (RefArray< TargetScriptDebugger >::iterator i = m_scriptDebuggers.begin(); i != m_scriptDebuggers.end(); ++i)
		listener->notifyDebuggerEndSession(*i);
	m_listeners.remove(listener);
	
}

	}
}

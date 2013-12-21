#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Amalgam/Editor/TargetScriptDebuggerSessions.h"
#include "Amalgam/Editor/TargetScriptProfiler.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetScriptDebuggerSessions", TargetScriptDebuggerSessions, script::IScriptDebuggerSessions)

void TargetScriptDebuggerSessions::beginSession(TargetScriptDebugger* scriptDebugger, TargetScriptProfiler* scriptProfiler)
{
	Session session;
	session.debugger = scriptDebugger;
	session.profiler = scriptProfiler;
	m_sessions.push_back(session);

	if (scriptDebugger)
	{
		for (std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
		{
			for (std::set< Guid >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
				scriptDebugger->setBreakpoint(*j, i->first);
		}
	}

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyBeginSession(scriptDebugger, scriptProfiler);
}

void TargetScriptDebuggerSessions::endSession(TargetScriptDebugger* scriptDebugger, TargetScriptProfiler* scriptProfiler)
{
	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyEndSession(scriptDebugger, scriptProfiler);

	if (scriptDebugger)
	{
		for (std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
		{
			for (std::set< Guid >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
				scriptDebugger->removeBreakpoint(*j, i->first);
		}
	}

	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
	{
		if (i->debugger == scriptDebugger && i->profiler == scriptProfiler)
		{
			m_sessions.erase(i);
			break;
		}
	}
}

bool TargetScriptDebuggerSessions::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	m_breakpoints[lineNumber].insert(scriptId);

	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		i->debugger->setBreakpoint(scriptId, lineNumber);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifySetBreakpoint(scriptId, lineNumber);

	return true;
}

bool TargetScriptDebuggerSessions::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	m_breakpoints[lineNumber].erase(scriptId);

	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		i->debugger->removeBreakpoint(scriptId, lineNumber);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyRemoveBreakpoint(scriptId, lineNumber);

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
	
	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		listener->notifyBeginSession(i->debugger, i->profiler);

	for (std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
	{
		for (std::set< Guid >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			listener->notifySetBreakpoint(*j, i->first);
	}

	m_listeners.push_back(listener);
}

void TargetScriptDebuggerSessions::removeListener(IListener* listener)
{
	T_ASSERT (listener);
	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		listener->notifyEndSession(i->debugger, i->profiler);
	m_listeners.remove(listener);
	
}

	}
}

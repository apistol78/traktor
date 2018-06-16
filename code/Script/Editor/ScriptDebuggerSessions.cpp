/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Guid.h"
#include "Script/IScriptDebugger.h"
#include "Script/IScriptProfiler.h"
#include "Script/Editor/ScriptDebuggerSessions.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerSessions", ScriptDebuggerSessions, IScriptDebuggerSessions)

void ScriptDebuggerSessions::beginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
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

void ScriptDebuggerSessions::endSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
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

bool ScriptDebuggerSessions::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	m_breakpoints[lineNumber].insert(scriptId);

	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		i->debugger->setBreakpoint(scriptId, lineNumber);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifySetBreakpoint(scriptId, lineNumber);

	return true;
}

bool ScriptDebuggerSessions::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	m_breakpoints[lineNumber].erase(scriptId);

	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		i->debugger->removeBreakpoint(scriptId, lineNumber);

	for (std::list< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->notifyRemoveBreakpoint(scriptId, lineNumber);

	return true;
}

bool ScriptDebuggerSessions::removeAllBreakpoints(const Guid& scriptId)
{
	for (std::map< int32_t, std::set< Guid > >::iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
	{
		int32_t lineNumber = i->first;

		std::set< Guid >::iterator it = i->second.find(scriptId);
		if (it != i->second.end())
		{
			for (std::list< Session >::iterator k = m_sessions.begin(); k != m_sessions.end(); ++k)
				k->debugger->removeBreakpoint(scriptId, lineNumber);

			for (std::list< IListener* >::iterator k = m_listeners.begin(); k != m_listeners.end(); ++k)
				(*k)->notifyRemoveBreakpoint(scriptId, lineNumber);

			i->second.erase(it);
		}
	}
	return true;
}

bool ScriptDebuggerSessions::haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const
{
	std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.find(lineNumber);
	if (i != m_breakpoints.end())
		return i->second.find(scriptId) != i->second.end();
	else
		return false;
}

void ScriptDebuggerSessions::addListener(IListener* listener)
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

void ScriptDebuggerSessions::removeListener(IListener* listener)
{
	T_ASSERT (listener);
	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		listener->notifyEndSession(i->debugger, i->profiler);
	m_listeners.remove(listener);
	
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Script/Editor/ScriptDebuggerSessions.h"

#include "Core/Guid.h"
#include "Script/IScriptDebugger.h"
#include "Script/IScriptProfiler.h"

namespace traktor::script
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
		for (std::map< int32_t, std::set< std::wstring > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
			for (const auto& fileName : i->second)
				scriptDebugger->setBreakpoint(fileName, i->first);
	}

	for (auto listener : m_listeners)
		listener->notifyBeginSession(scriptDebugger, scriptProfiler);
}

void ScriptDebuggerSessions::endSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	for (auto listener : m_listeners)
		listener->notifyEndSession(scriptDebugger, scriptProfiler);

	if (scriptDebugger)
	{
		for (std::map< int32_t, std::set< std::wstring > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
			for (const auto& fileName : i->second)
				scriptDebugger->removeBreakpoint(fileName, i->first);
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

bool ScriptDebuggerSessions::setBreakpoint(const std::wstring& fileName, int32_t lineNumber)
{
	m_breakpoints[lineNumber].insert(fileName);

	for (const auto& session : m_sessions)
		session.debugger->setBreakpoint(fileName, lineNumber);

	for (auto listener : m_listeners)
		listener->notifySetBreakpoint(fileName, lineNumber);

	return true;
}

bool ScriptDebuggerSessions::removeBreakpoint(const std::wstring& fileName, int32_t lineNumber)
{
	m_breakpoints[lineNumber].erase(fileName);

	for (const auto& session : m_sessions)
		session.debugger->removeBreakpoint(fileName, lineNumber);

	for (auto listener : m_listeners)
		listener->notifyRemoveBreakpoint(fileName, lineNumber);

	return true;
}

bool ScriptDebuggerSessions::removeAllBreakpoints(const std::wstring& fileName)
{
	for (std::map< int32_t, std::set< std::wstring > >::iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
	{
		int32_t lineNumber = i->first;

		std::set< std::wstring >::iterator it = i->second.find(fileName);
		if (it != i->second.end())
		{
			for (const auto& session : m_sessions)
				session.debugger->removeBreakpoint(fileName, lineNumber);

			for (auto listener : m_listeners)
				listener->notifyRemoveBreakpoint(fileName, lineNumber);

			i->second.erase(it);
		}
	}
	return true;
}

bool ScriptDebuggerSessions::haveBreakpoint(const std::wstring& fileName, int32_t lineNumber) const
{
	const auto it = m_breakpoints.find(lineNumber);
	if (it != m_breakpoints.end())
		return it->second.find(fileName) != it->second.end();
	else
		return false;
}

void ScriptDebuggerSessions::addListener(IListener* listener)
{
	T_ASSERT(listener);

	for (const auto& session : m_sessions)
		listener->notifyBeginSession(session.debugger, session.profiler);

	for (std::map< int32_t, std::set< std::wstring > >::const_iterator i = m_breakpoints.begin(); i != m_breakpoints.end(); ++i)
		for (std::set< std::wstring >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			listener->notifySetBreakpoint(*j, i->first);

	m_listeners.push_back(listener);
}

void ScriptDebuggerSessions::removeListener(IListener* listener)
{
	T_ASSERT(listener);
	for (std::list< Session >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		listener->notifyEndSession(i->debugger, i->profiler);
	m_listeners.remove(listener);
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <map>
#include "Core/Guid.h"
#include "Core/Class/Boxed.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Timer.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Script/Value.h"
#include "Script/ValueObject.h"
#include "Script/Variable.h"
#include "Script/StackFrame.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor::script
{
	namespace
	{

const int32_t c_tableKey_instance = -2;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerLua", ScriptDebuggerLua, IScriptDebugger)

ScriptDebuggerLua::ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_state(State::Running)
{
}

ScriptDebuggerLua::~ScriptDebuggerLua()
{
}

bool ScriptDebuggerLua::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_breakpoints[lineNumber].insert(scriptId);
	return true;
}

bool ScriptDebuggerLua::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_breakpoints[lineNumber].erase(scriptId);
	return true;
}

void ScriptDebuggerLua::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.insert(listener);
}

void ScriptDebuggerLua::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.erase(listener);

	// If all listeners have been removed then automatically continue running;
	// don't want the application to be kept locking up running thread.
	if (m_listeners.empty())
		m_state = State::Running;
}

bool ScriptDebuggerLua::captureStackFrame(uint32_t depth, Ref< StackFrame >& outStackFrame)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return false;

	lua_Debug ar = { 0 };
	if (!lua_getstack(currentContext->m_luaState, depth, &ar))
		return false;

	lua_getinfo(currentContext->m_luaState, "Snlu", &ar);

	outStackFrame = new StackFrame();
	outStackFrame->setScriptId(Guid(mbstows(ar.source)));
	outStackFrame->setFunctionName(ar.name ? mbstows(ar.name) : L"(anonymous)");
	outStackFrame->setLine(max(ar.currentline - 1, 0));

	return true;
}

bool ScriptDebuggerLua::captureLocals(uint32_t depth, RefArray< Variable >& outLocals)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return false;

	lua_State* L = currentContext->m_luaState;
	T_ANONYMOUS_VAR(UnwindStack)(L);

	lua_Debug ar = { 0 };
	if (!lua_getstack(L, depth, &ar))
		return false;

	// Get information about locals.
	const char* localName;
	for (int n = 1; (localName = lua_getlocal(L, &ar, n)) != 0; ++n)
	{
		if (*localName != '(')
		{
			Ref< Variable > variable = new Variable(mbstows(localName), L"", 0);

			if (lua_isnumber(L, -1))
			{
				variable->setTypeName(L"(number)");
				variable->setValue(new Value(toString(lua_tonumber(L, -1))));
				lua_pop(L, 1);
			}
			else if (lua_isboolean(L, -1))
			{
				variable->setTypeName(L"(boolean)");
				variable->setValue(new Value(lua_toboolean(L, -1) != 0 ? L"true" : L"false"));
				lua_pop(L, 1);
			}
			else if (lua_isstring(L, -1))
			{
				variable->setTypeName(L"(string)");
				variable->setValue(new Value(mbstows(lua_tostring(L, -1))));
				lua_pop(L, 1);
			}
			else if (lua_isfunction(L, -1))
			{
				variable->setTypeName(L"(function)");
				lua_pop(L, 1);
			}
			else if (lua_istable(L, -1))
			{
				lua_rawgeti(L, -1, c_tableKey_instance);
				if (lua_islightuserdata(L, -1))
				{
					ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(L, -1));
					lua_pop(L, 1);

					if (object)
						variable->setTypeName(type_name(object));

					const uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

					Boxed* b = dynamic_type_cast< Boxed* >(object);
					if (b)
						variable->setValue(new ValueObject(objectRef, b->toString()));
					else
						variable->setValue(new ValueObject(objectRef));
				}
				else
				{
					lua_pop(L, 1);

					const uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
					variable->setValue(new ValueObject(objectRef));
				}
			}
			else
				lua_pop(L, 1);

			outLocals.push_back(variable);
		}
		else
			lua_pop(L, 1);
	}

	// Get information about upvalues.
	if (lua_getinfo(L, "f", &ar) != 0)
	{
		const char* upName;
		for (int n = 1; (upName = lua_getupvalue(L, -1, n)) != 0; ++n)
		{
			if (*upName != '(')
			{
				Ref< Variable > variable = new Variable(mbstows(upName), L"", 0);

				if (lua_isnumber(L, -1))
				{
					variable->setTypeName(L"(number)");
					variable->setValue(new Value(toString(lua_tonumber(L, -1))));
					lua_pop(L, 1);
				}
				else if (lua_isboolean(L, -1))
				{
					variable->setTypeName(L"(boolean)");
					variable->setValue(new Value(lua_toboolean(L, -1) != 0 ? L"true" : L"false"));
					lua_pop(L, 1);
				}
				else if (lua_isstring(L, -1))
				{
					variable->setTypeName(L"(string)");
					variable->setValue(new Value(mbstows(lua_tostring(L, -1))));
					lua_pop(L, 1);
				}
				else if (lua_isfunction(L, -1))
				{
					variable->setTypeName(L"(function)");
					lua_pop(L, 1);
				}
				else if (lua_istable(L, -1))
				{
					lua_rawgeti(L, -1, c_tableKey_instance);
					if (lua_islightuserdata(L, -1))
					{
						ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(L, -1));
						lua_pop(L, 1);

						if (object)
							variable->setTypeName(type_name(object));

						const uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

						Boxed* b = dynamic_type_cast< Boxed* >(object);
						if (b)
							variable->setValue(new ValueObject(objectRef, b->toString()));
						else
							variable->setValue(new ValueObject(objectRef));
					}
					else
					{
						lua_pop(L, 1);

						const uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
						variable->setValue(new ValueObject(objectRef));
					}
				}
				else
					lua_pop(L, 1);

				outLocals.push_back(variable);
			}
			else
				lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	return true;
}

bool ScriptDebuggerLua::captureObject(uint32_t object, RefArray< Variable >& outMembers)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return false;

	lua_State* L = currentContext->m_luaState;
	T_ANONYMOUS_VAR(UnwindStack)(L);

	lua_rawgeti(L, LUA_REGISTRYINDEX, object);
	T_ASSERT(lua_istable(L, -1));

	if (lua_getmetatable(L, -1))
	{
		Ref< Variable > variable = new Variable(L"(meta)", L"", 0);
		const uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
		variable->setValue(new ValueObject(objectRef));
		outMembers.push_back(variable);
	}

	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		std::wstring name;

		if (lua_isnumber(L, -2))
			name = toString(lua_tonumber(L, -2));
		else if (lua_isboolean(L, -2))
			name = lua_toboolean(L, -2) != 0 ? L"true" : L"false";
		else if (lua_isstring(L, -2))
			name = mbstows(lua_tostring(L, -2));
		else
		{
			lua_pop(L, 1);
			continue;
		}

		if (name == L"-1" || name == L"-2")
		{
			lua_pop(L, 1);
			continue;
		}

		Ref< Variable > variable = new Variable(name, L"", 0);

		if (lua_isnumber(L, -1))
		{
			variable->setTypeName(L"(number)");
			variable->setValue(new Value(toString(lua_tonumber(L, -1))));
			lua_pop(L, 1);
		}
		else if (lua_isboolean(L, -1))
		{
			variable->setTypeName(L"(boolean)");
			variable->setValue(new Value(lua_toboolean(L, -1) != 0 ? L"true" : L"false"));
			lua_pop(L, 1);
		}
		else if (lua_isstring(L, -1))
		{
			variable->setTypeName(L"(string)");
			variable->setValue(new Value(mbstows(lua_tostring(L, -1))));
			lua_pop(L, 1);
		}
		else if (lua_isfunction(L, -1))
		{
			variable->setTypeName(L"(function)");
			lua_pop(L, 1);
		}
		else if (lua_istable(L, -1))
		{
			lua_rawgeti(L, -1, c_tableKey_instance);
			if (lua_islightuserdata(L, -1))
			{
				ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(L, -1));
				lua_pop(L, 1);

				if (object)
					variable->setTypeName(type_name(object));
			}
			else
				lua_pop(L, 1);

			const uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			variable->setValue(new ValueObject(objectRef));
		}
		else
			lua_pop(L, 1);

		outMembers.push_back(variable);
	}

	return true;
}

bool ScriptDebuggerLua::captureBreadcrumbs(AlignedVector< uint32_t >& outBreadcrumbs)
{
	outBreadcrumbs.resize(m_breadcrumb.size());
	for (uint32_t i = 0; i < m_breadcrumb.size(); ++i)
		outBreadcrumbs[i] = m_breadcrumb[i];
	return true;
}

bool ScriptDebuggerLua::isRunning() const
{
	return m_state == State::Running;
}

bool ScriptDebuggerLua::actionBreak()
{
	m_state = State::Break;
	return true;
}

bool ScriptDebuggerLua::actionContinue()
{
	m_state = State::Running;
	return true;
}

bool ScriptDebuggerLua::actionStepInto()
{
	m_state = State::StepInto;
	return false;
}

bool ScriptDebuggerLua::actionStepOver()
{
	m_state = State::StepOver;
	return false;
}

void ScriptDebuggerLua::analyzeState(lua_State* L, lua_Debug* ar)
{
	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return;

	const int32_t currentLine = ar->currentline - 1;
	m_breadcrumb.push_back(currentLine);

	if (m_state == State::Running)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Any breakpoint defined with current line number?
		auto it = m_breakpoints.find(currentLine);
		if (it != m_breakpoints.end())
		{
			// Get executing script's identifier.
			lua_getinfo(L, "S", ar);
			if (currentId.create(mbstows(ar->source)))
			{
				// If script identifier also match then we're in the right script
				// so we halt and trigger "breakpoint reached".
				if (it->second.find(currentId) != it->second.end())
				{
					m_state = State::Halted;
					m_lastId = currentId;

					for (auto listener : m_listeners)
						listener->debugeeStateChange(this);
				}
			}
		}
	}
	else if (m_state == State::Break)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			m_state = State::Halted;
			m_lastId = currentId;
			for (auto listener : m_listeners)
				listener->debugeeStateChange(this);
		}
	}
	else if (m_state == State::StepInto)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			m_state = State::Halted;
			m_lastId = currentId;
			for (auto listener : m_listeners)
				listener->debugeeStateChange(this);
		}
	}
	else if (m_state == State::StepOver)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			if (currentId == m_lastId)
			{
				m_state = State::Halted;
				m_lastId = currentId;
				for (auto listener : m_listeners)
					listener->debugeeStateChange(this);
			}
		}
	}

	if (m_state == State::Halted)
	{
		// Wait until state is no longer halted.
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		while (m_state == State::Halted && !m_listeners.empty() && !currentThread->stopped())
			currentThread->sleep(100);

		if (m_state == State::Halted)
			m_state = State::Running;

		m_breadcrumb.clear();

		for (auto listener : m_listeners)
			listener->debugeeStateChange(this);
	}
}

void ScriptDebuggerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	if (ar->event == LUA_HOOKLINE)
		analyzeState(L, ar);
}

}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <map>
#include "Core/Guid.h"
#include "Core/Class/Boxes.h"
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

namespace traktor
{
	namespace script
	{
		namespace
		{

const int32_t c_tableKey_instance = -2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerLua", ScriptDebuggerLua, IScriptDebugger)

ScriptDebuggerLua::ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_state(StRunning)
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
		m_state = StRunning;
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

					uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

					Boxed* b = dynamic_type_cast< Boxed* >(object);
					if (b)
						variable->setValue(new ValueObject(objectRef, b->toString()));
					else
						variable->setValue(new ValueObject(objectRef));
				}
				else
				{
					lua_pop(L, 1);

					uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
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

						uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

						Boxed* b = dynamic_type_cast< Boxed* >(object);
						if (b)
							variable->setValue(new ValueObject(objectRef, b->toString()));
						else
							variable->setValue(new ValueObject(objectRef));
					}
					else
					{
						lua_pop(L, 1);

						uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
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
	T_ASSERT (lua_istable(L, -1));

	if (lua_getmetatable(L, -1))
	{
		Ref< Variable > variable = new Variable(L"(meta)", L"", 0);
		uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
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

			uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			variable->setValue(new ValueObject(objectRef));
		}
		else
			lua_pop(L, 1);

		outMembers.push_back(variable);
	}

	return true;
}

bool ScriptDebuggerLua::isRunning() const
{
	return m_state == StRunning;
}

bool ScriptDebuggerLua::actionBreak()
{
	m_state = StBreak;
	return true;
}

bool ScriptDebuggerLua::actionContinue()
{
	m_state = StRunning;
	return true;
}

bool ScriptDebuggerLua::actionStepInto()
{
	m_state = StStepInto;
	return false;
}

bool ScriptDebuggerLua::actionStepOver()
{
	m_state = StStepOver;
	return false;
}

void ScriptDebuggerLua::analyzeState(lua_State* L, lua_Debug* ar)
{
	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return;

	if (m_state == StRunning)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		int32_t currentLine = ar->currentline - 1;
		Guid currentId;

		// Any breakpoint defined with current line number?
		SmallMap< int32_t, SmallSet< Guid > >::const_iterator i = m_breakpoints.find(currentLine);
		if (i != m_breakpoints.end())
		{
			// Get executing script's identifier.
			lua_getinfo(L, "S", ar);
			if (currentId.create(mbstows(ar->source)))
			{
				// If identifier also match then we halt and trigger "breakpoint reached".
				if (i->second.find(currentId) != i->second.end())
				{
					m_state = StHalted;
					m_lastId = currentId;

					for (SmallSet< IListener* >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
						(*j)->debugeeStateChange(this);
				}

			}
		}
	}
	else if (m_state == StBreak)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			m_state = StHalted;
			m_lastId = currentId;
			for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->debugeeStateChange(this);
		}
	}
	else if (m_state == StStepInto)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			m_state = StHalted;
			m_lastId = currentId;
			for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->debugeeStateChange(this);
		}
	}
	else if (m_state == StStepOver)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			if (currentId == m_lastId)
			{
				m_state = StHalted;
				m_lastId = currentId;
				for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
					(*i)->debugeeStateChange(this);
			}
		}
	}

	if (m_state == StHalted)
	{
		// Wait until state is no longer halted.
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do
		{
			currentThread->sleep(100);
		}
		while (m_state == StHalted && !currentThread->stopped());

		for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->debugeeStateChange(this);
	}
}

void ScriptDebuggerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	if (ar->event == LUA_HOOKLINE)
		analyzeState(L, ar);
}

	}
}

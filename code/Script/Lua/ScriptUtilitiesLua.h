/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptUtilitiesLua_H
#define traktor_script_ScriptUtilitiesLua_H

#if defined(_DEBUG)
#	define T_SCRIPT_LUA_FORCE_STACK_CHECK 1
#else
#	define T_SCRIPT_LUA_FORCE_STACK_CHECK 0
#endif

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#if T_SCRIPT_LUA_FORCE_STACK_CHECK
#	include "Core/Io/StringOutputStream.h"
#endif

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lstate.h"
}

namespace traktor
{

class OutputStream;

	namespace script
	{

#if T_SCRIPT_LUA_FORCE_STACK_CHECK
class CheckStack
{
public:
	CheckStack(lua_State* luaState, int32_t expectedOffset = 0)
	:	m_luaState(luaState)
	,	m_top(lua_gettop(luaState) + expectedOffset)
	{
	}

	~CheckStack()
	{
		int32_t top = lua_gettop(m_luaState);
		if (m_top != top)
		{
			StringOutputStream ss;
			ss << L"LUA stack unbalanced, entering with " << m_top << L", leaving with " << top;
			T_FATAL_ASSERT_M (m_top == top, ss.str().c_str());
		}
	}

private:
	lua_State* m_luaState;
	int32_t m_top;
};
#	define CHECK_LUA_STACK(state, expectedOffset) CheckStack cs(state, expectedOffset);
#else
#	define CHECK_LUA_STACK(state, expectedOffset)
#endif

class UnwindStack
{
public:
	UnwindStack(lua_State* luaState)
	:	m_luaState(luaState)
	,	m_top(lua_gettop(luaState))
	{
	}

	~UnwindStack()
	{
		int32_t npop = lua_gettop(m_luaState) - m_top;
		T_FATAL_ASSERT (npop >= 0);
		lua_pop(m_luaState, npop);
	}

private:
	lua_State* m_luaState;
	int32_t m_top;
};

void dumpStack(lua_State* luaState, OutputStream& os, int32_t base = 1);

#if defined(_DEBUG)
#	define DUMP_LUA_STACK(state) stackDump(state)
#else
#	define DUMP_LUA_STACK(state)
#endif

int luaPrint(lua_State *L);

int luaSleep(lua_State* L);

	}
}

#endif	// traktor_script_ScriptUtilitiesLua_H

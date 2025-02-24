/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined(_DEBUG)
#	define T_SCRIPT_LUA_FORCE_STACK_CHECK	1
#	define T_SCRIPT_LUA_DUMP_STACK			0
#else
#	define T_SCRIPT_LUA_FORCE_STACK_CHECK	0
#	define T_SCRIPT_LUA_DUMP_STACK			0
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

}

namespace traktor::script
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

void dumpStack(lua_State* luaState, OutputStream& os, int32_t base = 0);

#if T_SCRIPT_LUA_DUMP_STACK
#	define DUMP_LUA_STACK(state) dumpStack(state, log::info)

#	define __DO__W0(x) L ## x
#	define __DO__W1(x) __DO__W0(#x)

#	define DO_0(state, statement)					\
		DUMP_LUA_STACK(state);						\
		log::info << __DO__W1(statement) << Endl;	\
		(statement);								\
		DUMP_LUA_STACK(state);						\

#	define DO_1(state, statement)					\
		log::info << __DO__W1(statement) << Endl;	\
		(statement);								\
		DUMP_LUA_STACK(state);

#else
#	define DUMP_LUA_STACK(state)
#	define DO_0(state, statement)	(statement);
#	define DO_1(state, statement)	(statement);
#endif

int luaPrint(lua_State *L);

int luaSleep(lua_State* L);

}

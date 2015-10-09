#ifndef traktor_script_ScriptUtilitiesLua_H
#define traktor_script_ScriptUtilitiesLua_H

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace traktor
{

class OutputStream;

	namespace script
	{

#if defined(_DEBUG)
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
		T_ASSERT (m_top == top);
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
		T_ASSERT (lua_gettop(m_luaState) >= m_top);
		lua_pop(m_luaState, lua_gettop(m_luaState) - m_top);
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

void* luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize);

int luaPrint(lua_State *L);

	}
}

#endif	// traktor_script_ScriptUtilitiesLua_H

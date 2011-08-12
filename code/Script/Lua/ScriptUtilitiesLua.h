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

void stackDump(lua_State* luaState);

#if defined(_DEBUG)
#	define DUMP_LUA_STACK(state) stackDump(state)
#else
#	define DUMP_LUA_STACK(state)
#endif

void* luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize);

size_t luaAllocTotal();

int luaPrint(lua_State *L);

	}
}

#endif	// traktor_script_ScriptUtilitiesLua_H

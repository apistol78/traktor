/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cstring>
#include "Core/Io/OutputStream.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

void dumpStack(lua_State* luaState, OutputStream& os, int32_t base)
{
	int32_t top = lua_gettop(luaState);
	for (int32_t i = base; i <= top; ++i)
	{
		int t = lua_type(luaState, i);
		switch (t)
		{
		case LUA_TSTRING:
			os << i << L".\tstring: \"" << mbstows(lua_tostring(luaState, i)) << L"\"" << Endl;
			break;

		case LUA_TBOOLEAN:
			os << i << L".\tboolean: " << (lua_toboolean(luaState, i) ? L"true" : L"false") << Endl;
			break;

		case LUA_TNUMBER:
			os << i << L".\tnumber: " << lua_tonumber(luaState, i) << Endl;
			break;

		case LUA_TTABLE:
			os << i << L".\ttable" << Endl;

			lua_getfield(luaState, i, "__name");
			if (lua_isstring(luaState, -1))
				os << i << L"\t\t__name \"" << mbstows(lua_tostring(luaState, -1)) << L"\"" << Endl;
			lua_pop(luaState, 1);

			break;

		default:  /* other values */
			os << i << L".\tother: " << mbstows(lua_typename(luaState, t)) << Endl;
			break;
		}
	}
}

int luaPrint(lua_State* L)
{
	int n = lua_gettop(L);
	int i;

	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; ++i)
	{
		const char* s;
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		if (i > 1)
			log::info << L"\t";
		log::info << mbstows(s);
		lua_pop(L, 1);
	}

	log::info << Endl;
	return 0;
}

	}
}

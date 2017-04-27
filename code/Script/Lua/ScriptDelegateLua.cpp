/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDelegateLua", ScriptDelegateLua, IRuntimeDelegate)

ScriptDelegateLua::ScriptDelegateLua(ScriptContextLua* context, lua_State*& luaState)
:	m_context(context)
,	m_luaState(luaState)
,	m_functionRef(0)
{
	m_functionRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
}

ScriptDelegateLua::~ScriptDelegateLua()
{
	if (m_luaState)
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
}

void ScriptDelegateLua::push()
{
	T_ASSERT (m_luaState);
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
	T_ASSERT (lua_isfunction(m_luaState, -1));
}

Any ScriptDelegateLua::call(int32_t argc, const Any* argv)
{
	return m_context->executeDelegate(this, argc, argv);
}

	}
}

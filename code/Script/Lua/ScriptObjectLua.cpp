/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptObjectLua", ScriptObjectLua, IRuntimeObject)

ScriptObjectLua::ScriptObjectLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, int32_t tableRef)
:	m_scriptManager(scriptManager)
,	m_scriptContext(scriptContext)
,	m_luaState(luaState)
,	m_tableRef(tableRef)
{
}

ScriptObjectLua::~ScriptObjectLua()
{
	luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
}

Ref< const IRuntimeClass > ScriptObjectLua::getRuntimeClass() const
{
	T_ANONYMOUS_VAR(UnwindStack)(m_luaState);
	push();
	lua_getmetatable(m_luaState, -1);
	return ScriptClassLua::createFromStack(m_scriptManager, m_scriptContext, m_luaState);
}

void ScriptObjectLua::push() const
{
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
	T_ASSERT (lua_istable(m_luaState, -1));
}

	}
}

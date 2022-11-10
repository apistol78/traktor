/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

	}
}

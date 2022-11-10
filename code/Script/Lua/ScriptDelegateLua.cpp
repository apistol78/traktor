/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDelegateLua", ScriptDelegateLua, IRuntimeDelegate)

ScriptDelegateLua::ScriptDelegateLua(ScriptContextLua* context, lua_State*& luaState, int32_t functionRef)
:	m_context(context)
,	m_luaState(luaState)
,	m_functionRef(functionRef)
{
}

ScriptDelegateLua::~ScriptDelegateLua()
{
	if (m_luaState)
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
}

void ScriptDelegateLua::push()
{
	T_ASSERT(m_luaState);
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
	T_ASSERT(lua_isfunction(m_luaState, -1));
}

Any ScriptDelegateLua::call(int32_t argc, const Any* argv)
{
	return m_context->executeDelegate(this, argc, argv);
}

	}
}

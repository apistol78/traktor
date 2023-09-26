/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/IRuntimeDelegate.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor::script
{

class ScriptContextLua;

/*! LUA script delegate runtime interface.
 * \ingroup Script
 */
class ScriptDelegateLua : public IRuntimeDelegate
{
	T_RTTI_CLASS;

public:
	explicit ScriptDelegateLua(ScriptContextLua* context, lua_State*& luaState, int32_t functionRef);

	virtual ~ScriptDelegateLua();

	/*! Push script delegate onto LUA stack. */
	void push() const
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
		T_ASSERT(lua_isfunction(m_luaState, -1));
	}

	virtual Any call(int32_t argc, const Any* argv) override final;

private:
	ScriptContextLua* m_context;
	lua_State*& m_luaState;
	int32_t m_tableRef;
	int32_t m_functionRef;
};

}

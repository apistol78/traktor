/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/IRuntimeObject.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor::script
{

/*! LUA script object runtime interface.
 * \ingroup Script
 */
class ScriptObjectLua : public IRuntimeObject
{
	T_RTTI_CLASS;

public:
	explicit ScriptObjectLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, int32_t tableRef);

	virtual ~ScriptObjectLua();

	virtual Ref< const IRuntimeClass > getRuntimeClass() const override final;

	/*! Push script object onto LUA stack. */
	void push() const
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
		T_ASSERT(lua_istable(m_luaState, -1));
	}

private:
	ScriptManagerLua* m_scriptManager;
	ScriptContextLua* m_scriptContext;
	lua_State*& m_luaState;
	int32_t m_tableRef;
};

}

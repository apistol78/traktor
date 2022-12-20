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

struct lua_State;

namespace traktor::script
{

class ScriptContextLua;

class ScriptDelegateLua : public IRuntimeDelegate
{
	T_RTTI_CLASS;

public:
	explicit ScriptDelegateLua(ScriptContextLua* context, lua_State*& luaState, int32_t functionRef);

	virtual ~ScriptDelegateLua();

	void push();

	virtual Any call(int32_t argc, const Any* argv) override final;

private:
	ScriptContextLua* m_context;
	lua_State*& m_luaState;
	int32_t m_tableRef;
	int32_t m_functionRef;
};

}

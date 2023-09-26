/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallSet.h"
#include "Script/IScriptContext.h"

struct lua_State;

namespace traktor::script
{

class ScriptDelegateLua;
class ScriptManagerLua;
class ScriptObjectLua;

/*! LUA scripting context.
 * \ingroup Script
 */
class ScriptContextLua : public IScriptContext
{
	T_RTTI_CLASS;

public:
	virtual ~ScriptContextLua();

	virtual void destroy() override final;

	virtual bool load(const IScriptBlob* scriptBlob) override final;

	virtual void setGlobal(const std::string& globalName, const Any& globalValue) override final;

	virtual Any getGlobal(const std::string& globalName) override final;

	virtual Ref< const IRuntimeClass > findClass(const std::string& className) override final;

	virtual bool haveFunction(const std::string& functionName) const override final;

	virtual Any executeFunction(const std::string& functionName, uint32_t argc, const Any* argv) override final;

	Any executeDelegate(ScriptDelegateLua* delegate, uint32_t argc, const Any* argv);

	Any executeMethod(ScriptObjectLua* self, int32_t methodRef, uint32_t argc, const Any* argv);

private:
	friend class ScriptClassLua;
	friend class ScriptDebuggerLua;
	friend class ScriptDelegateLua;
	friend class ScriptManagerLua;
	friend class ScriptProfilerLua;

	ScriptManagerLua* m_scriptManager;
	lua_State* m_luaState;
	int32_t m_environmentRef;
	bool m_strict;
	const Object* m_lastSelf;
	SmallSet< std::string > m_globals;

	ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState, int32_t environmentRef, bool strict);

	static int32_t runtimeError(lua_State* luaState);

	static int32_t permitGlobalWrite(lua_State* luaState);

	static int32_t restrictedAccessWrite(lua_State* luaState);

	static int32_t restrictedAccessRead(lua_State* luaState);
};

}

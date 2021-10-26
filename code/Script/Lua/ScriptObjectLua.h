#pragma once

#include "Core/Class/IRuntimeObject.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

class ScriptObjectLua : public IRuntimeObject
{
	T_RTTI_CLASS;

public:
	explicit ScriptObjectLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, int32_t tableRef);

	virtual ~ScriptObjectLua();

	virtual Ref< const IRuntimeClass > getRuntimeClass() const override final;

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
}


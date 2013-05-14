#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDelegateLua", ScriptDelegateLua, IScriptDelegate)

ScriptDelegateLua::ScriptDelegateLua(ScriptManagerLua* manager, ScriptContextLua* context, lua_State* luaState)
:	m_manager(manager)
,	m_context(context)
,	m_luaState(luaState)
,	m_functionRef(0)
{
	m_functionRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
}

ScriptDelegateLua::~ScriptDelegateLua()
{
	luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
}

void ScriptDelegateLua::push()
{
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
	T_ASSERT (lua_isfunction(m_luaState, -1));
}

Any ScriptDelegateLua::call(int32_t argc, const Any* argv)
{
	Any returnValue;
	m_manager->lock(m_context);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		push();

		for (uint32_t i = 0; i < argc; ++i)
			m_manager->pushAny(argv[i]);

		int32_t err = lua_pcall(m_luaState, argc, 1, 0);
		if (err == 0)
			returnValue = m_manager->toAny(-1);

		lua_pop(m_luaState, 1);
	}
	m_manager->unlock();
	return returnValue;
}

	}
}

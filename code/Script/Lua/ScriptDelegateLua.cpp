#include "Core/Log/Log.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

int32_t s_delegateCount = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDelegateLua", ScriptDelegateLua, IScriptDelegate)

ScriptDelegateLua::ScriptDelegateLua(ScriptManagerLua* manager, ScriptContextLua* context, lua_State*& luaState)
:	m_manager(manager)
,	m_context(context)
,	m_luaState(luaState)
,	m_functionRef(0)
,	m_tag(s_delegateCount++)
{
	m_functionRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
	T_DEBUG(L"Delegate " << m_tag << L" created (" << s_delegateCount << L")");
}

ScriptDelegateLua::~ScriptDelegateLua()
{
	--s_delegateCount;
	if (m_luaState)
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
	T_DEBUG(L"Delegate " << m_tag << L" destroyed (" << s_delegateCount << L")");
}

void ScriptDelegateLua::push()
{
	T_ASSERT (m_luaState);
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_functionRef);
	T_ASSERT (lua_isfunction(m_luaState, -1));
}

Any ScriptDelegateLua::call(int32_t argc, const Any* argv)
{
	T_ASSERT (m_luaState);
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

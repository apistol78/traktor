#include "Core/Log/Log.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

int32_t s_delegateCount = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDelegateLua", ScriptDelegateLua, IRuntimeDelegate)

ScriptDelegateLua::ScriptDelegateLua(ScriptContextLua* context, lua_State*& luaState)
:	m_context(context)
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
	return m_context->executeDelegate(this, argc, argv);
}

	}
}

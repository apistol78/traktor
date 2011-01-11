#include "Core/Log/Log.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptResourceLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextLua", ScriptContextLua, IScriptContext)

ScriptContextLua::ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
{
	CHECK_LUA_STACK(m_luaState, 0)

	// Create local environment.
	lua_newtable(m_luaState);
	m_environmentRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);

	// Create table with __index as global environment.
	lua_newtable(m_luaState);
	lua_getglobal(m_luaState, "_G");
	lua_setfield(m_luaState, -2, "__index");

	// Setup "inheritance" with the global environment.
	lua_setmetatable(m_luaState, -2);
	lua_pop(m_luaState, 1);
}

ScriptContextLua::~ScriptContextLua()
{
	destroy();
}

void ScriptContextLua::destroy()
{
	// Unpin our local environment reference.
	if (m_environmentRef != LUA_NOREF)
	{
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		m_environmentRef = LUA_NOREF;

		// Perform a full garbage collect; don't want
		// lingering objects.
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
	}
}

void ScriptContextLua::setGlobal(const std::wstring& globalName, const Any& globalValue)
{
	CHECK_LUA_STACK(m_luaState, 0);
	
	m_scriptManager->lock(this);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	m_scriptManager->pushAny(globalValue);
	lua_setfield(m_luaState, -2, wstombs(globalName).c_str());

	m_scriptManager->unlock();
}

Any ScriptContextLua::getGlobal(const std::wstring& globalName)
{
	CHECK_LUA_STACK(m_luaState, 0);

	m_scriptManager->lock(this);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	lua_getfield(m_luaState, -1, wstombs(globalName).c_str());
	Any value = m_scriptManager->toAny(-1);

	m_scriptManager->unlock();
	return value;
}

bool ScriptContextLua::executeScript(const IScriptResource* scriptResource, const Guid& scriptGuid)
{
	CHECK_LUA_STACK(m_luaState, 0);

	m_scriptManager->lock(this);

	const ScriptResourceLua* scriptResourceLua = checked_type_cast< const ScriptResourceLua*, false >(scriptResource);
	
	const std::string& text = scriptResourceLua->getScript();
	int32_t result = luaL_loadbuffer(
		m_luaState,
		text.c_str(),
		text.length(),
		wstombs(scriptGuid.format()).c_str()
	);

	if (result != 0)
	{
		lua_pop(m_luaState, 1);
		m_scriptManager->unlock();
		return false;
	}

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	lua_setfenv(m_luaState, -2);
	lua_call(m_luaState, 0, 0);

	m_scriptManager->unlock();

	return true;
}

bool ScriptContextLua::haveFunction(const std::wstring& functionName) const
{
	CHECK_LUA_STACK(m_luaState, 0);

	m_scriptManager->lock((ScriptContextLua*)this);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	lua_getfield(m_luaState, -1, wstombs(functionName).c_str());
	bool result = (lua_isfunction(m_luaState, -1) != 0);
	lua_pop(m_luaState, 2);

	m_scriptManager->unlock();
	return result;
}

Any ScriptContextLua::executeFunction(const std::wstring& functionName, uint32_t argc, const Any* argv)
{
	CHECK_LUA_STACK(m_luaState, 0);

	m_scriptManager->lock(this);

	Any returnValue;
	if (m_scriptManager->setPanicJump())
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, wstombs(functionName).c_str());

		if (lua_isfunction(m_luaState, -1))
		{
			for (uint32_t i = 0; i < argc; ++i)
				m_scriptManager->pushAny(argv[i]);
			lua_call(m_luaState, argc, 1);
			returnValue = m_scriptManager->toAny(-1);
		}

		lua_pop(m_luaState, 2);
	}

	m_scriptManager->unlock();
	return returnValue;
}

Any ScriptContextLua::executeMethod(Object* self, const std::wstring& methodName, uint32_t argc, const Any* argv)
{
	CHECK_LUA_STACK(m_luaState, 0);

	m_scriptManager->lock(this);

	Any returnValue;
	if (m_scriptManager->setPanicJump())
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, wstombs(methodName).c_str());

		if (lua_isfunction(m_luaState, -1))
		{
			m_scriptManager->pushAny(Any(self));
			for (uint32_t i = 0; i < argc; ++i)
				m_scriptManager->pushAny(argv[i]);
			lua_call(m_luaState, int32_t(argc + 1), 1);
			returnValue = m_scriptManager->toAny(-1);
		}

		lua_pop(m_luaState, 2);
	}

	m_scriptManager->unlock();
	return returnValue;
}

	}
}

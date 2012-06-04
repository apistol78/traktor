#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptResourceLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

std::wstring translateSource(const source_map_t& map, int32_t line)
{
	StringOutputStream ss;
	for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
	{
		if (line >= i->first)
		{
			ss << i->second << L"(" << (line - i->first + 1) << L")";
			return ss.str();
		}
	}
	ss << L"< No source >(" << (line + 1) << L")";
	return ss.str();
}

void translateError(const char* error, const source_map_t& map)
{
	if (error)
	{
		WildCompare wc(L"[*]:*:*");
		std::vector< std::wstring > pieces;
		if (wc.match(mbstows(error), WildCompare::CmIgnoreCase, &pieces))
		{
			int32_t line = parseString< int32_t >(pieces[1]) - 1;
			log::error << L"Lua runtime error: " << translateSource(map, line) << L": " << trim(pieces[2]) << Endl;
		}
		else
			log::error << L"Lua runtime error: " << error << Endl;
	}
	else
		log::error << L"Unknown lua runtime error" << Endl;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextLua", ScriptContextLua, IScriptContext)

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

		/*
		// Perform a full garbage collect; don't want
		// lingering objects.
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
		*/
	}
}

void ScriptContextLua::setGlobal(const std::wstring& globalName, const Any& globalValue)
{
	CHECK_LUA_STACK(m_luaState, 0);
	
	m_scriptManager->lock(this);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	m_scriptManager->pushAny(globalValue);
	lua_setfield(m_luaState, -2, wstombs(globalName).c_str());
	lua_pop(m_luaState, 1);

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

	Any returnValue;

	m_scriptManager->lock(this);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	lua_getfield(m_luaState, -1, wstombs(functionName).c_str());

	if (lua_isfunction(m_luaState, -1))
	{
		for (uint32_t i = 0; i < argc; ++i)
			m_scriptManager->pushAny(argv[i]);
		int32_t err = lua_pcall(m_luaState, argc, 1, 0);
		if (err == 0)
			returnValue = m_scriptManager->toAny(-1);
		else
		{
			const char* err = lua_tostring(m_luaState, lua_gettop(m_luaState));
			translateError(err, m_map);
		}
	}

	lua_pop(m_luaState, 2);

	m_scriptManager->unlock();
	return returnValue;
}

Any ScriptContextLua::executeMethod(Object* self, const std::wstring& methodName, uint32_t argc, const Any* argv)
{
	CHECK_LUA_STACK(m_luaState, 0);

	m_scriptManager->lock(this);

	Any returnValue;

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
	lua_getfield(m_luaState, -1, wstombs(methodName).c_str());

	if (lua_isfunction(m_luaState, -1))
	{
		// Set _self variable in global environment.
		m_scriptManager->pushAny(Any(self));
		lua_setglobal(m_luaState, "_self");

		// Push arguments.
		for (uint32_t i = 0; i < argc; ++i)
			m_scriptManager->pushAny(argv[i]);
		
		// Call script function.
		int32_t err = lua_pcall(m_luaState, argc, 1, 0);
		if (err == 0)
			returnValue = m_scriptManager->toAny(-1);
		else
		{
			const char* err = lua_tostring(m_luaState, lua_gettop(m_luaState));
			translateError(err, m_map);
		}
	}

	lua_pop(m_luaState, 2);

	m_scriptManager->unlock();
	return returnValue;
}

ScriptContextLua::ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState, int32_t environmentRef, const source_map_t& map)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_environmentRef(environmentRef)
,	m_map(map)
{
}

	}
}

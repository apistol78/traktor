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
		if (line >= i->line)
		{
			ss << i->name << L"(" << (line - i->line + 1) << L")";
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
			log::error << L"Lua runtime error: " << mbstows(error) << Endl;
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
	if (m_scriptManager)
	{
		m_scriptManager->lock(this);
		{
			// Unpin our local environment reference.
			if (m_environmentRef != LUA_NOREF)
			{
				luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
				m_environmentRef = LUA_NOREF;
				m_luaState =  0;
			}

			// Perform a full garbage collect; don't want
			// lingering objects.
			m_scriptManager->collectGarbageFull();
			m_scriptManager->destroyContext(this);
		}
		m_scriptManager->unlock();
		m_scriptManager = 0;
	}
}

void ScriptContextLua::setGlobal(const std::string& globalName, const Any& globalValue)
{
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		m_scriptManager->pushAny(globalValue);
		lua_setfield(m_luaState, -2, globalName.c_str());
		lua_pop(m_luaState, 1);
	}
	m_scriptManager->unlock();
}

Any ScriptContextLua::getGlobal(const std::string& globalName)
{
	Any value;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, globalName.c_str());

		value = m_scriptManager->toAny(-1);
	}
	m_scriptManager->unlock();
	return value;
}

bool ScriptContextLua::haveFunction(const std::string& functionName) const
{
	bool result;
	m_scriptManager->lock((ScriptContextLua*)this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, functionName.c_str());
		
		result = (lua_isfunction(m_luaState, -1) != 0);
		
		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return result;
}

Any ScriptContextLua::executeFunction(const std::string& functionName, uint32_t argc, const Any* argv)
{
	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, functionName.c_str());

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
		else
			log::error << L"Unable to call " << mbstows(functionName) << L"; no such function" << Endl;

		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return returnValue;
}

Any ScriptContextLua::executeMethod(Object* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, methodName.c_str());

		if (lua_isfunction(m_luaState, -1))
		{
			// Set "self" variable in global environment.
			m_scriptManager->pushAny(Any::fromObject(self));
			lua_setglobal(m_luaState, "self");

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
		else
			log::error << L"Unable to call " << mbstows(methodName) << L"; no such method" << Endl;

		lua_pop(m_luaState, 2);
	}
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

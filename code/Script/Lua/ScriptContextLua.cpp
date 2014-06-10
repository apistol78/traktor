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

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextLua", ScriptContextLua, IScriptContext)

ScriptContextLua::~ScriptContextLua()
{
	destroy();
}

void ScriptContextLua::destroy()
{
	if (m_scriptManager)
	{
		// Store reference locally as later the garbage
		// collect might recurse this call.
		Ref< ScriptManagerLua > scriptManager = m_scriptManager;
		m_scriptManager = 0;

		scriptManager->lock(this);
		{
			// Unpin our local environment reference.
			if (m_environmentRef != LUA_NOREF)
			{
				// Clear all global variables first.
				lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
				lua_pushnil(m_luaState);

				// -2 = environmentRef
				// -1 = nil
				while (lua_next(m_luaState, -2))
				{
					// -3 = environmentRef
					// -2 = key
					// -1 = value
					lua_pop(m_luaState, 1);

					// -2 = environmentRef
					// -1 = key
					lua_pushvalue(m_luaState, -1);

					// -3 = environmentRef
					// -2 = key
					// -1 = key
					lua_pushnil(m_luaState);

					// -4 = environmentRef
					// -3 = key
					// -2 = key
					// -1 = nil
					lua_rawset(m_luaState, -4);

					// -2 = environmentRef
					// -1 = key
				}

				luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
				m_environmentRef = LUA_NOREF;
				m_luaState =  0;
			}

			// Perform a full garbage collect; don't want
			// lingering objects.
			scriptManager->collectGarbageFull();
			scriptManager->collectGarbageFull();
			scriptManager->destroyContext(this);
		}
		scriptManager->unlock();
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

		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		int32_t errfunc = lua_gettop(m_luaState);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, functionName.c_str());

		if (lua_isfunction(m_luaState, -1))
		{
			for (uint32_t i = 0; i < argc; ++i)
				m_scriptManager->pushAny(argv[i]);

			int32_t err = lua_pcall(m_luaState, argc, 1, errfunc);
			if (err == 0)
				returnValue = m_scriptManager->toAny(-1);
		}
		else
			log::error << L"Unable to call " << mbstows(functionName) << L"; no such function" << Endl;

		lua_pop(m_luaState, 3);
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

		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		int32_t errfunc = lua_gettop(m_luaState);

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
			int32_t err = lua_pcall(m_luaState, argc, 1, errfunc);
			if (err == 0)
				returnValue = m_scriptManager->toAny(-1);
		}
		else
			log::error << L"Unable to call " << mbstows(methodName) << L"; no such method" << Endl;

		lua_pop(m_luaState, 3);
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

int32_t ScriptContextLua::runtimeError(lua_State* luaState)
{
	ScriptContextLua* this_ = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (this_);
	T_ASSERT (this_->m_scriptManager);
	log::error << L"LUA RUNTIME ERROR; Debugger halted if attached." << Endl;

	std::wstring error = mbstows(lua_tostring(luaState, -1));
	
	const SourceMapping* errorMap = 0;
	std::wstring errorMessage;
	int32_t errorLine;

	if (error.size() >= 43 && error[0] == L'{' && error[42] == L':')
	{
		size_t pos = error.find(L':', 43);
		if (pos != error.npos)
		{
			errorMessage = error.substr(pos + 1);
			errorLine = parseString< int32_t >(error.substr(43, pos - 43));

			const source_map_t& map = this_->m_map;
			for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
			{
				if (errorLine >= i->line)
				{
					errorMap = &(*i);
					errorLine = errorLine - i->line;
					break;
				}
			}
		}
	}

	if (errorMap)
		log::error << errorMap->name << L":" << errorLine << L":" << errorMessage << Endl;
	else
		log::error << error << Endl;

	this_->m_scriptManager->breakDebugger(luaState);
	return 0;
}

	}
}

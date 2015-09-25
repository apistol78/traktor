#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Script/Lua/ScriptBlobLua.h"
#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptObjectLua.h"
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
			scriptManager->collectGarbageFullNoLock();
			scriptManager->collectGarbageFullNoLock();
			scriptManager->destroyContext(this);
		}
		scriptManager->unlock();
	}
}

bool ScriptContextLua::load(const IScriptBlob* scriptBlob)
{
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		const ScriptBlobLua* scriptBlobLua = mandatory_non_null_type_cast< const ScriptBlobLua* >(scriptBlob);
		int32_t result = luaL_loadbuffer(
			m_luaState,
			(const char*)scriptBlobLua->m_script.c_str(),
			scriptBlobLua->m_script.length(),
			scriptBlobLua->m_fileName.c_str()
		);
		if (result != 0)
		{
			log::error << L"Script context load resource failed; \"" << mbstows(lua_tostring(m_luaState, -1)) << L"\"" << Endl;
			lua_pop(m_luaState, 1);
			m_scriptManager->unlock();
			return false;
		}

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
#if defined(T_LUA_5_2)
		lua_setupvalue(m_luaState, -2, 1);
		lua_call(m_luaState, 0, 0);
#else
		lua_setfenv(m_luaState, -2);
		lua_call(m_luaState, 0, 0);
#endif
	}
	m_scriptManager->unlock();
	return true;
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

Ref< const IRuntimeClass > ScriptContextLua::findClass(const std::string& className)
{
	Ref< ScriptClassLua > scriptClass;

	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, className.c_str());
		if (lua_istable(m_luaState, -1))
		{
			scriptClass = new ScriptClassLua(m_scriptManager, this, m_luaState, className);

			// Gather all methods of script class.
			lua_pushnil(m_luaState);
			while (lua_next(m_luaState, -2))
			{
				if (lua_isfunction(m_luaState, -1))
				{
					const char* functionName = lua_tostring(m_luaState, -2);
					T_ASSERT (functionName);

					lua_pushvalue(m_luaState, -1);
					int32_t functionRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

					scriptClass->addMethod(functionName, functionRef);
				}
				lua_pop(m_luaState, 1);
			}
		}
		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();

	return scriptClass;
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
			// Push arguments.
			{
				CHECK_LUA_STACK(m_luaState, argc);
				for (int32_t i = 0; i < argc; ++i)
				{
					const Any& any = argv[i];
					if (any.isVoid())
						lua_pushnil(m_luaState);
					else if (any.isBoolean())
						lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
					else if (any.isInteger())
						lua_pushinteger(m_luaState, any.getIntegerUnsafe());
					else if (any.isFloat())
						lua_pushnumber(m_luaState, any.getFloatUnsafe());
					else if (any.isString())
						lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
					else if (any.isObject())
						m_scriptManager->pushObject(any.getObjectUnsafe());
					else
						lua_pushnil(m_luaState);
				}
			}

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

Any ScriptContextLua::executeDelegate(ScriptDelegateLua* delegate, uint32_t argc, const Any* argv)
{
	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		// Push error function.
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		int32_t errfunc = lua_gettop(m_luaState);

		delegate->push();

		// Push arguments.
		{
			CHECK_LUA_STACK(m_luaState, argc);
			for (int32_t i = 0; i < argc; ++i)
			{
				const Any& any = argv[i];
				if (any.isVoid())
					lua_pushnil(m_luaState);
				else if (any.isBoolean())
					lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
				else if (any.isInteger())
					lua_pushinteger(m_luaState, any.getIntegerUnsafe());
				else if (any.isFloat())
					lua_pushnumber(m_luaState, any.getFloatUnsafe());
				else if (any.isString())
					lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
				else if (any.isObject())
					m_scriptManager->pushObject(any.getObjectUnsafe());
				else
					lua_pushnil(m_luaState);
			}
		}

		int32_t err = lua_pcall(m_luaState, argc, 1, errfunc);
		if (err == 0)
			returnValue = m_scriptManager->toAny(-1);

		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return returnValue;
}

Any ScriptContextLua::executeMethod(ScriptObjectLua* self, int32_t methodRef, uint32_t argc, const Any* argv)
{
	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		// Push error function.
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		int32_t errfunc = lua_gettop(m_luaState);

		// Push LUA function to call.
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, methodRef);

		// Push wrapped LUA object.
		if (self)
			self->push();

		// Push arguments.
		{
			CHECK_LUA_STACK(m_luaState, argc);
			for (int32_t i = 0; i < argc; ++i)
			{
				const Any& any = argv[i];
				if (any.isVoid())
					lua_pushnil(m_luaState);
				else if (any.isBoolean())
					lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
				else if (any.isInteger())
					lua_pushinteger(m_luaState, any.getIntegerUnsafe());
				else if (any.isFloat())
					lua_pushnumber(m_luaState, any.getFloatUnsafe());
				else if (any.isString())
					lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
				else if (any.isObject())
					m_scriptManager->pushObject(any.getObjectUnsafe());
				else
					lua_pushnil(m_luaState);
			}
		}
		
		// Call script function.
		int32_t err = lua_pcall(m_luaState, argc + (self ? 1 : 0), 1, errfunc);
		if (err == 0)
			returnValue = m_scriptManager->toAny(-1);

		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return returnValue;
}

ScriptContextLua::ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState, int32_t environmentRef)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_environmentRef(environmentRef)
,	m_lastSelf(0)
{
}

int32_t ScriptContextLua::runtimeError(lua_State* luaState)
{
	ScriptContextLua* this_ = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (this_);
	T_ASSERT (this_->m_scriptManager);

	log::error << L"LUA RUNTIME ERROR; Debugger halted if attached." << Endl;

	std::wstring error = mbstows(lua_tostring(luaState, -1));
	if (!error.empty())
		log::error << error << Endl;

	this_->m_scriptManager->breakDebugger(luaState);
	return 0;
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Timer/Profiler.h"
#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class ScriptClassConstructorDispatch : public IRuntimeDispatch
{
public:
	ScriptClassConstructorDispatch(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, int32_t classRef, int32_t constructorRef)
	:	m_scriptManager(scriptManager)
	,	m_scriptContext(scriptContext)
	,	m_luaState(luaState)
	,	m_classRef(classRef)
	,	m_constructorRef(constructorRef)
	{
	}

	virtual ~ScriptClassConstructorDispatch()
	{
		if (m_constructorRef)
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_constructorRef);
		if (m_classRef)
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_classRef);
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_PROFILER_SCOPE(L"Script invoke");

		// Allocate table for script side object.
		if (self)
			m_scriptManager->pushObject(self);
		else
			lua_newtable(m_luaState);

		// Associate class with script side object.
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_classRef);
		lua_setmetatable(m_luaState, -2);

		// Create instance table.
		int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

		// Create C++ script object.
		Ref< ScriptObjectLua > scriptSelf = new ScriptObjectLua(m_scriptManager, m_scriptContext, m_luaState, tableRef);

		// Prepend "self" object as first in arguments.
		Any argv2[16];
		argv2[0] = Any::fromObject(scriptSelf);
		for (uint32_t i = 0; i < argc; ++i)
			argv2[i + 1] = argv[i];

		// Call constructor method in script land.
		if (m_constructorRef)
		{
			m_scriptContext->executeMethod(
				0,
				m_constructorRef,
				argc + 1,
				argv2
			);
		}

		return Any::fromObject(scriptSelf);
	}

private:
	ScriptManagerLua* m_scriptManager;
	ScriptContextLua* m_scriptContext;
	lua_State*& m_luaState;
	int32_t m_classRef;
	int32_t m_constructorRef;
};

class ScriptClassMethodDispatch : public IRuntimeDispatch
{
public:
	ScriptClassMethodDispatch(ScriptContextLua* scriptContext, lua_State*& luaState, int32_t ref)
	:	m_scriptContext(scriptContext)
	,	m_luaState(luaState)
	,	m_ref(ref)
	{
	}

	virtual ~ScriptClassMethodDispatch()
	{
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_ref);
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_PROFILER_SCOPE(L"Script invoke");
		return m_scriptContext->executeMethod(
			mandatory_non_null_type_cast< ScriptObjectLua* >(self),
			m_ref,
			argc,
			argv
		);
	}

private:
	ScriptContextLua* m_scriptContext;
	lua_State*& m_luaState;
	int32_t m_ref;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptClassLua", ScriptClassLua, IRuntimeClass)

Ref< ScriptClassLua > ScriptClassLua::createFromStack(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState)
{
	T_FATAL_ASSERT(lua_istable(luaState, -1));

	Ref< ScriptClassLua > sc = new ScriptClassLua(scriptManager, scriptContext, luaState);

	int32_t classRef = luaL_ref(luaState, LUA_REGISTRYINDEX);
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, classRef);

	int32_t constructorRef = 0;

	lua_pushnil(luaState);
	while (lua_next(luaState, -2))
	{
		if (lua_isfunction(luaState, -1))
		{
			std::string functionName = lua_tostring(luaState, -2);

			lua_pushvalue(luaState, -1);
			int32_t functionRef = luaL_ref(luaState, LUA_REGISTRYINDEX);

			if (functionName != "new")
			{
				Method& m = sc->m_methods.push_back();
				m.name = functionName;
				m.dispatch = new ScriptClassMethodDispatch(scriptContext, luaState, functionRef);
			}
			else
			{
				constructorRef = functionRef;
			}
		}
		lua_pop(luaState, 1);
	}

	sc->m_constructor = new ScriptClassConstructorDispatch(scriptManager, scriptContext, luaState, classRef, constructorRef);

	return sc;
}

const TypeInfo& ScriptClassLua::getExportType() const
{
	return type_of< Object >();
}

const IRuntimeDispatch* ScriptClassLua::getConstructorDispatch() const
{
	return m_constructor;
}

uint32_t ScriptClassLua::getConstantCount() const
{
	return 0;
}

std::string ScriptClassLua::getConstantName(uint32_t constId) const
{
	return "";
}

Any ScriptClassLua::getConstantValue(uint32_t constId) const
{
	return Any();
}

uint32_t ScriptClassLua::getMethodCount() const
{
	return uint32_t(m_methods.size());
}

std::string ScriptClassLua::getMethodName(uint32_t methodId) const
{
	return m_methods[methodId].name;
}

const IRuntimeDispatch* ScriptClassLua::getMethodDispatch(uint32_t methodId) const
{
	return m_methods[methodId].dispatch;
}

uint32_t ScriptClassLua::getStaticMethodCount() const
{
	return 0;
}

std::string ScriptClassLua::getStaticMethodName(uint32_t methodId) const
{
	return "";
}

const IRuntimeDispatch* ScriptClassLua::getStaticMethodDispatch(uint32_t methodId) const
{
	return 0;
}

uint32_t ScriptClassLua::getPropertiesCount() const
{
	return 0;
}

std::string ScriptClassLua::getPropertyName(uint32_t propertyId) const
{
	return "";
}

const IRuntimeDispatch* ScriptClassLua::getPropertyGetDispatch(uint32_t propertyId) const
{
	return 0;
}

const IRuntimeDispatch* ScriptClassLua::getPropertySetDispatch(uint32_t propertyId) const
{
	return 0;
}

const IRuntimeDispatch* ScriptClassLua::getUnknownDispatch() const
{
	return 0;
}

const IRuntimeDispatch* ScriptClassLua::getOperatorDispatch(OperatorType op) const
{
	return 0;
}

ScriptClassLua::ScriptClassLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState)
:	m_scriptManager(scriptManager)
,	m_scriptContext(scriptContext)
,	m_luaState(luaState)
{
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptClassLua", ScriptClassLua, IRuntimeClass)

ScriptClassLua::ScriptClassLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, const std::string& className)
:	m_scriptManager(scriptManager)
,	m_scriptContext(scriptContext)
,	m_luaState(luaState)
,	m_className(className)
{
}

ScriptClassLua::~ScriptClassLua()
{
	for (AlignedVector< Method >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
	{
		if (m_luaState)
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, i->ref);
	}
}

void ScriptClassLua::addMethod(const std::string& name, int32_t ref)
{
	Method& m = m_methods.push_back();
	m.name = name;
	m.ref = ref;
	m_methodLookup[name] = uint32_t(m_methods.size() - 1);
}

const TypeInfo& ScriptClassLua::getExportType() const
{
	return type_of< Object >();
}

bool ScriptClassLua::haveConstructor() const
{
	return false;
}

bool ScriptClassLua::haveUnknown() const
{
	return false;
}

Ref< ITypedObject > ScriptClassLua::construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto) const
{
	m_scriptManager->lock(m_scriptContext);

	// Allocate table for script side object.
	if (self)
		m_scriptManager->pushObject(self);
	else
		lua_newtable(m_luaState);

	// Create instance table.
	int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	// Create C++ script object.
	Ref< ScriptObjectLua > scriptSelf = new ScriptObjectLua(m_luaState, tableRef);

	// Initialize prototype members before calling constructor.
	for (prototype_t::const_iterator i = proto.begin(); i != proto.end(); ++i)
	{
		T_ANONYMOUS_VAR(UnwindStack)(m_luaState);
		scriptSelf->push();
		m_scriptManager->pushAny(i->second);
		lua_setfield(m_luaState, -2, i->first.c_str());
	}

	m_scriptManager->unlock();

	// Prepend "self" object as first in arguments.
	Any argv2[16];
	argv2[0] = Any::fromObject(scriptSelf);
	for (uint32_t i = 0; i < argc; ++i)
		argv2[i + 1] = argv[i];

	// Call constructor method in script land.
	SmallMap< std::string, uint32_t >::const_iterator i = m_methodLookup.find("new");
	if (i != m_methodLookup.end())
	{
		const Method& m = m_methods[i->second];
		m_scriptContext->executeMethod(
			0,
			m.ref,
			argc + 1,
			argv2
		);
	}

	return scriptSelf;
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

std::wstring ScriptClassLua::getMethodSignature(uint32_t methodId) const
{
	return L"";
}

Any ScriptClassLua::invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const
{
	T_PROFILER_SCOPE(L"Script invoke");
	return m_scriptContext->executeMethod(
		mandatory_non_null_type_cast< ScriptObjectLua* >(object),
		m_methods[methodId].ref,
		argc,
		argv
	);
}

uint32_t ScriptClassLua::getStaticMethodCount() const
{
	return 0;
}

std::string ScriptClassLua::getStaticMethodName(uint32_t methodId) const
{
	return "";
}

std::wstring ScriptClassLua::getStaticMethodSignature(uint32_t methodId) const
{
	return L"";
}

Any ScriptClassLua::invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const
{
	return Any();
}

uint32_t ScriptClassLua::getPropertiesCount() const
{
	return 0;
}

std::string ScriptClassLua::getPropertyName(uint32_t propertyId) const
{
	return "";
}

std::wstring ScriptClassLua::getPropertySignature(uint32_t propertyId) const
{
	return L"";
}

Any ScriptClassLua::invokePropertyGet(ITypedObject* self, uint32_t propertyId) const
{
	return Any();
}

void ScriptClassLua::invokePropertySet(ITypedObject* self, uint32_t propertyId, const Any& value) const
{
}

Any ScriptClassLua::invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const
{
	return Any();
}

Any ScriptClassLua::invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const
{
	return Any();
}

	}
}

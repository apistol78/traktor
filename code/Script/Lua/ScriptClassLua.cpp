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
	for (std::vector< Method >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
	{
		if (m_luaState)
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, i->ref);
	}
}

void ScriptClassLua::addMethod(const std::string& name, int32_t ref)
{
	Method m;
	m.name = name;
	m.ref = ref;
	m_methods.push_back(m);
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

Ref< ITypedObject > ScriptClassLua::construct(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	Any argv2[32];

	// Create a script object box for "self".
	m_scriptManager->lock(m_scriptContext);
	m_scriptManager->pushObject(self);
	int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
	Ref< ScriptObjectLua > scriptSelf = new ScriptObjectLua(m_luaState, tableRef, this);
	m_scriptManager->unlock();

	// Prepend "self" object first in arguments.
	argv2[0] = Any::fromObject(scriptSelf);
	for (uint32_t i = 0; i < argc; ++i)
		argv2[i + 1] = argv[i];

	// Call constructor method in script land.
	for (std::vector< Method >::const_iterator i = m_methods.begin(); i != m_methods.end(); ++i)
	{
		if (i->name == "new")
		{
			/*Any ret = */m_scriptContext->executeMethod(
				0,
				i->ref,
				argc + 1,
				argv2
			);
			//return ret.getObject();
			break;
		}
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

Any ScriptClassLua::invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const
{
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

Any ScriptClassLua::invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const
{
	return Any();
}

Any ScriptClassLua::invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const
{
	return Any();
}

Any ScriptClassLua::invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const
{
	return Any();
}

bool ScriptClassLua::getMember(ITypedObject* object, const std::string& memberName, Any& outValue) const
{
	return false;
}

	}
}

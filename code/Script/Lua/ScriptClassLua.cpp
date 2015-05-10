#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptClassLua", ScriptClassLua, IRuntimeClass)

ScriptClassLua::ScriptClassLua(ScriptContextLua* context, lua_State*& luaState, const std::string& className)
:	m_context(context)
,	m_luaState(luaState)
,	m_className(className)
{
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

Ref< ITypedObject > ScriptClassLua::construct(uint32_t argc, const Any* argv) const
{
	return 0;
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
	return m_context->executeMethod(
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

	}
}

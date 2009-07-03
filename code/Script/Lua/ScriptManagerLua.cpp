#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.script.ScriptManagerLua", ScriptManagerLua, IScriptManager)

void ScriptManagerLua::registerClass(IScriptClass* scriptClass)
{
	m_registeredClasses.push_back(scriptClass);
}

IScriptContext* ScriptManagerLua::createContext()
{
	return gc_new< ScriptContextLua >(cref(m_registeredClasses));
}

	}
}

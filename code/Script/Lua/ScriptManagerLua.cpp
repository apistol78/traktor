#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/IScriptClass.h"
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

IScriptClass* ScriptManagerLua::findScriptClass(const Type& type) const
{
	Ref< IScriptClass > minScriptClass;
	uint32_t minScriptClassDiff = ~0UL;

	for (RefArray< IScriptClass >::const_iterator i = m_registeredClasses.begin(); i != m_registeredClasses.end(); ++i)
	{
		uint32_t scriptClassDiff = type_difference((*i)->getExportType(), type);
		if (scriptClassDiff < minScriptClassDiff)
		{
			minScriptClass = *i;
			minScriptClassDiff = scriptClassDiff;
		}
	}

	return minScriptClass;
}

IScriptContext* ScriptManagerLua::createContext()
{
	return gc_new< ScriptContextLua >(cref(m_registeredClasses));
}

	}
}

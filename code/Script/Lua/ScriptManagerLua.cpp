#include "Core/Serialization/ISerializable.h"
#include "Script/Boxes.h"
#include "Script/IScriptClass.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptContextLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerLua", 0, ScriptManagerLua, IScriptManager)

ScriptManagerLua::ScriptManagerLua()
{
	registerBoxClasses(this);
}

void ScriptManagerLua::registerClass(IScriptClass* scriptClass)
{
	m_registeredClasses.push_back(scriptClass);
}

Ref< IScriptClass > ScriptManagerLua::findScriptClass(const TypeInfo& type) const
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

Ref< IScriptContext > ScriptManagerLua::createContext()
{
	return new ScriptContextLua(m_registeredClasses);
}

	}
}

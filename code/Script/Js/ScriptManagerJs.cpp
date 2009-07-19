#include <v8.h>
#define T_HAVE_TYPES
#include "Script/Js/ScriptManagerJs.h"
#include "Script/Js/ScriptContextJs.h"
#include "Script/IScriptClass.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.script.ScriptManagerJs", ScriptManagerJs, IScriptManager)

void ScriptManagerJs::registerClass(IScriptClass* scriptClass)
{
	m_registeredClasses.push_back(scriptClass);
}

IScriptClass* ScriptManagerJs::findScriptClass(const Type& type) const
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

IScriptContext* ScriptManagerJs::createContext()
{
	Ref< ScriptContextJs > scriptContext = gc_new< ScriptContextJs >();
	if (!scriptContext->create(m_registeredClasses))
		return 0;
	return scriptContext;
}

	}
}

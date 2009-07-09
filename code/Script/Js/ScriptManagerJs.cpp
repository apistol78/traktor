#include <v8.h>
#define T_HAVE_TYPES
#include "Script/Js/ScriptManagerJs.h"
#include "Script/Js/ScriptContextJs.h"
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

IScriptContext* ScriptManagerJs::createContext()
{
	Ref< ScriptContextJs > scriptContext = gc_new< ScriptContextJs >();
	if (!scriptContext->create(m_registeredClasses))
		return 0;
	return scriptContext;
}

	}
}

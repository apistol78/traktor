#include <v8.h>
#define T_HAVE_TYPES
#include "Core/Serialization/ISerializable.h"
#include "Script/Boxes.h"
#include "Script/IScriptClass.h"
#include "Script/Js/ScriptContextJs.h"
#include "Script/Js/ScriptManagerJs.h"
#include "Script/Js/ScriptResourceJs.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerJs", 0, ScriptManagerJs, IScriptManager)

ScriptManagerJs::ScriptManagerJs()
{
	registerBoxClasses(this);
}

void ScriptManagerJs::destroy()
{
}

void ScriptManagerJs::registerClass(IScriptClass* scriptClass)
{
	m_registeredClasses.push_back(scriptClass);
}

Ref< IScriptResource > ScriptManagerJs::compile(const std::wstring& fileName, const std::wstring& script, const source_map_t* map, IErrorCallback* errorCallback) const
{
	return new ScriptResourceJs(script);
}

Ref< IScriptContext > ScriptManagerJs::createContext(const IScriptResource* scriptResource, const IScriptContext* contextPrototype)
{
	Ref< ScriptContextJs > scriptContext = new ScriptContextJs();
	if (!scriptContext->create(m_registeredClasses, scriptResource))
		return 0;
	return scriptContext;
}

Ref< IScriptDebugger > ScriptManagerJs::createDebugger()
{
	return 0;
}

void ScriptManagerJs::collectGarbage()
{
}

	}
}

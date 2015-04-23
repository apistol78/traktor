#include <v8.h>
#define T_HAVE_TYPES
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializable.h"
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
}

void ScriptManagerJs::destroy()
{
}

void ScriptManagerJs::registerClass(IRuntimeClass* runtimeClass)
{
	m_registeredClasses.push_back(runtimeClass);
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

Ref< IScriptProfiler > ScriptManagerJs::createProfiler()
{
	return 0;
}

void ScriptManagerJs::collectGarbage(bool full)
{
}

void ScriptManagerJs::getStatistics(ScriptStatistics& outStatistics) const
{
	outStatistics.memoryUsage = 0;
}

	}
}

#include "Core/Serialization/ISerializer.h"
#include "Script/Js/ScriptResourceJs.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptResourceJs", 0, ScriptResourceJs, IScriptResource)

ScriptResourceJs::ScriptResourceJs()
{
}

ScriptResourceJs::ScriptResourceJs(const std::wstring& script)
:	m_script(script)
{
}

const std::wstring& ScriptResourceJs::getScript() const
{
	return m_script;
}

void ScriptResourceJs::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"script", m_script);
}

	}
}

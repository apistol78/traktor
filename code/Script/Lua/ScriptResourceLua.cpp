#include "Core/Serialization/ISerializer.h"
#include "Script/Lua/ScriptResourceLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptResourceLua", 0, ScriptResourceLua, IScriptResource)

ScriptResourceLua::ScriptResourceLua()
{
}

ScriptResourceLua::ScriptResourceLua(const std::wstring& script)
:	m_script(script)
{
}

const std::wstring& ScriptResourceLua::getScript() const
{
	return m_script;
}

bool ScriptResourceLua::serialize(ISerializer& s)
{
	return s >> Member< std::wstring >(L"script", m_script);
}

	}
}

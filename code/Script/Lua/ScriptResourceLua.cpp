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

ScriptResourceLua::ScriptResourceLua(const std::string& script)
:	m_script(script)
{
}

const std::string& ScriptResourceLua::getScript() const
{
	return m_script;
}

bool ScriptResourceLua::serialize(ISerializer& s)
{
	return s >> Member< std::string >(L"script", m_script);
}

	}
}

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/Lua/ScriptResourceLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class MemberSourceMapping : public MemberComplex
{
public:
	MemberSourceMapping(const wchar_t* const name, SourceMapping& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< Guid >(L"id", m_ref.id);
		s >> Member< std::wstring >(L"name", m_ref.name);
		s >> Member< int32_t >(L"line", m_ref.line);
	}

private:
	SourceMapping& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptResourceLua", 0, ScriptResourceLua, IScriptResource)

ScriptResourceLua::ScriptResourceLua()
{
}

ScriptResourceLua::ScriptResourceLua(const std::string& fileName, const std::string& script, const source_map_t& map)
:	m_fileName(fileName)
,	m_script(script)
,	m_map(map)
{
}

const std::string& ScriptResourceLua::getFileName() const
{
	return m_fileName;
}

const std::string& ScriptResourceLua::getScript() const
{
	return m_script;
}

const source_map_t& ScriptResourceLua::getMap() const
{
	return m_map;
}

void ScriptResourceLua::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"fileName", m_fileName);
	s >> Member< std::string >(L"script", m_script);
	s >> MemberStlList< SourceMapping, MemberSourceMapping >(L"map", m_map);
}

	}
}

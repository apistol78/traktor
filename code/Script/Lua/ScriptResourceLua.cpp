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
:	m_precompiled(false)
,	m_bufferSize(0)
{
}

void ScriptResourceLua::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"fileName", m_fileName);
	s >> MemberStlList< SourceMapping, MemberSourceMapping >(L"map", m_map);
	s >> Member< bool >(L"precompiled", m_precompiled);
	s >> Member< uint32_t >(L"bufferSize", m_bufferSize);

	if (s.getDirection() == ISerializer::SdRead)
		m_buffer.reset(new uint8_t [m_bufferSize]);

	s >> Member< void* >(L"buffer", m_buffer.ptr(), m_bufferSize);
}

	}
}

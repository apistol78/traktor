#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/IScriptContext.h"
#include "Script/Editor/Script.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class MemberDependency : public Member< Guid >
{
public:
	MemberDependency(const wchar_t* const name, Member< Guid >::value_type& ref)
	:	Member< Guid >(name, ref, AttributeType(type_of< Script >()))
	{
	}
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.script.Script", 1, Script, ISerializable)

Script::Script()
{
}

Script::Script(const std::wstring& text)
:	m_text(text)
{
}

void Script::addDependency(const Guid& dependency)
{
	m_dependencies.push_back(dependency);
}

std::vector< Guid >& Script::getDependencies()
{
	return m_dependencies;
}

const std::vector< Guid >& Script::getDependencies() const
{
	return m_dependencies;
}

void Script::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Script::getText() const
{
	return m_text;
}

void Script::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberStlVector< Guid, MemberDependency >(L"dependencies", m_dependencies);
	s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine());
}

	}
}

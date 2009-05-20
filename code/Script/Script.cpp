#include "Script/Script.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class MemberDependency : public Member< Guid >
{
public:
	MemberDependency(const std::wstring& name, Member< Guid >::value_type& ref)
	:	Member< Guid >(name, ref, &type_of< Script >())
	{
	}
};

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.script.Script", Script, Serializable)

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

int Script::getVersion() const
{
	return 1;
}

bool Script::serialize(Serializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberStlVector< Guid, MemberDependency >(L"dependencies", m_dependencies);
	return s >> Member< std::wstring >(L"text", m_text, true);
}

	}
}

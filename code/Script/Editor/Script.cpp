#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/IScriptResource.h"
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
	MemberDependency(const std::wstring& name, Member< Guid >::value_type& ref)
	:	Member< Guid >(name, ref, AttributeType(type_of< Script >()))
	{
	}
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.script.Script", 1, Script, editor::ITypedAsset)

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

const TypeInfo* Script::getOutputType() const
{
	return &type_of< IScriptResource >();
}

bool Script::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberStlVector< Guid, MemberDependency >(L"dependencies", m_dependencies);
	return s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine());
}

	}
}

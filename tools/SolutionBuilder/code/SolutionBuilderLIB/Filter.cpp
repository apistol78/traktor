#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include "Filter.h"

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Filter", 0, Filter, ProjectItem)

void Filter::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Filter::getName() const
{
	return m_name;
}

void Filter::serialize(traktor::ISerializer& s)
{
	s >> traktor::Member< std::wstring >(L"name", m_name);
	ProjectItem::serialize(s);
}

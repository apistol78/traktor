#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include "Filter.h"

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"Filter", Filter, ProjectItem)

void Filter::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Filter::getName() const
{
	return m_name;
}

bool Filter::serialize(traktor::Serializer& s)
{
	s >> traktor::Member< std::wstring >(L"name", m_name);
	return ProjectItem::serialize(s);
}

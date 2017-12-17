/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "SolutionBuilder/Filter.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Filter", 0, Filter, ProjectItem)

void Filter::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Filter::getName() const
{
	return m_name;
}

void Filter::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	ProjectItem::serialize(s);
}

	}
}

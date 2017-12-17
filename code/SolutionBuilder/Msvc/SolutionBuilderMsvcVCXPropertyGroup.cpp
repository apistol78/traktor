/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXPropertyGroup.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXPropertyGroup", 1, SolutionBuilderMsvcVCXPropertyGroup, ISerializable)

bool SolutionBuilderMsvcVCXPropertyGroup::generate(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	OutputStream& os
) const
{
	if (m_condition.empty())
		os << L"<PropertyGroup Label=\"" << m_label << L"\">" << Endl;
	else
		os << L"<PropertyGroup Label=\"" << m_label << L"\" Condition=\"" << m_condition << L"\">" << Endl;

	os << IncreaseIndent;
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
		os << L"<" << i->first << L">" << i->second << L"</" << i->first << L">" << Endl;
	os << DecreaseIndent;

	os << L"</PropertyGroup>" << Endl;
	return true;
}

void SolutionBuilderMsvcVCXPropertyGroup::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"label", m_label);
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"condition", m_condition);
	s >> MemberStlMap< std::wstring, std::wstring >(L"values", m_values);
}

	}
}

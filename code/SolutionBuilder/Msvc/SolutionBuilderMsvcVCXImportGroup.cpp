/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImportGroup.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXImportGroup", 0, SolutionBuilderMsvcVCXImportGroup, SolutionBuilderMsvcVCXImportCommon)

SolutionBuilderMsvcVCXImportGroup::SolutionBuilderMsvcVCXImportGroup()
{
}

SolutionBuilderMsvcVCXImportGroup::SolutionBuilderMsvcVCXImportGroup(const std::wstring& label, const std::wstring& condition)
:	m_label(label)
,	m_condition(condition)
{
}

void SolutionBuilderMsvcVCXImportGroup::addImport(SolutionBuilderMsvcVCXImportCommon* import)
{
	m_imports.push_back(import);
}

bool SolutionBuilderMsvcVCXImportGroup::generate(OutputStream& os) const
{
	os << L"<ImportGroup";
	if (!m_label.empty())
		os << L" Label=\"" << m_label << L"\"";
	if (!m_condition.empty())
		os << L" Condition=\"" << m_condition << L"\"";
	os << L">" << Endl;
	os << IncreaseIndent;

	for (RefArray< SolutionBuilderMsvcVCXImportCommon >::const_iterator i = m_imports.begin(); i != m_imports.end(); ++i)
		(*i)->generate(os);

	os << DecreaseIndent;
	os << L"</ImportGroup>" << Endl;
	return true;
}

void SolutionBuilderMsvcVCXImportGroup::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"label", m_label);
	s >> Member< std::wstring >(L"condition", m_condition);
	s >> MemberRefArray< SolutionBuilderMsvcVCXImportCommon >(L"imports", m_imports);
}

	}
}

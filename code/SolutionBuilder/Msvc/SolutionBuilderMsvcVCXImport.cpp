/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImport.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXImport", 0, SolutionBuilderMsvcVCXImport, SolutionBuilderMsvcVCXImportCommon)

SolutionBuilderMsvcVCXImport::SolutionBuilderMsvcVCXImport()
{
}

SolutionBuilderMsvcVCXImport::SolutionBuilderMsvcVCXImport(
	const std::wstring& label,
	const std::wstring& project,
	const std::wstring& condition
)
:	m_label(label)
,	m_project(project)
,	m_condition(condition)
{
}

bool SolutionBuilderMsvcVCXImport::generate(OutputStream& os) const
{
	os << L"<Import Project=\"" << m_project << L"\" ";
	if (!m_condition.empty())
		os << L"Condition=\"" << m_condition << L"\" ";
	if (!m_label.empty())
		os << L"Label=\"" << m_label << L"\" ";
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcVCXImport::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"label", m_label);
	s >> Member< std::wstring >(L"project", m_project);
	s >> Member< std::wstring >(L"condition", m_condition);
}

	}
}

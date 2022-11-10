/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

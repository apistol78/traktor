/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXPropertyGroup.h"

namespace traktor::sb
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
	for (auto it : m_values)
		os << L"<" << it.first << L">" << it.second << L"</" << it.first << L">" << Endl;
	os << DecreaseIndent;

	os << L"</PropertyGroup>" << Endl;
	return true;
}

void SolutionBuilderMsvcVCXPropertyGroup::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 1);
	s >> Member< std::wstring >(L"label", m_label);
	s >> Member< std::wstring >(L"condition", m_condition);
	s >> MemberStlMap< std::wstring, std::wstring >(L"values", m_values);
}

}

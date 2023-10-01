/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Output.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"Output", Output, Object)

Output::Output(const std::vector< std::wstring >& sections)
:	m_sections(sections)
,	m_ss(1 * 1024 * 1024)
{
}

void Output::print(const std::wstring& str)
{
	m_ss << str;
}

void Output::printLn(const std::wstring& str)
{
	m_ss << str << Endl;
}

void Output::printSection(int32_t id)
{
	T_FATAL_ASSERT(id >= 0 && id < int32_t(m_sections.size()));
	m_ss << m_sections[id];
}

std::wstring Output::getProduct() const
{
	return m_ss.str();
}

}

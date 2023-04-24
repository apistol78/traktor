/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Run/App/ProduceOutput.h"

namespace traktor::run
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.ProduceOutput", ProduceOutput, IOutput)

void ProduceOutput::print(const std::wstring& s)
{
	m_ss << s;
}

void ProduceOutput::printLn(const std::wstring& s)
{
	m_ss << s << Endl;
}

void ProduceOutput::printSection(int32_t id)
{
	T_ASSERT(id >= 0 && id < int32_t(m_sections.size()));
	m_ss << m_sections[id];
}

int32_t ProduceOutput::addSection(const std::wstring& section)
{
	m_sections.push_back(section);
	return int32_t(m_sections.size()) - 1;
}

std::wstring ProduceOutput::getProduct() const
{
	return m_ss.str();
}

}

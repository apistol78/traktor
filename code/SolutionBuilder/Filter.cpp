/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Filter.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.Filter", 0, Filter, ProjectItem)

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

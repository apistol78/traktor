/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <string>
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

MemberComplex::MemberComplex(const wchar_t* const name, bool compound)
:	m_name(name)
,	m_compound(compound)
,	m_attributes(0)
{
}

MemberComplex::MemberComplex(const wchar_t* const name, bool compound, const Attribute& attributes)
:	m_name(name)
,	m_compound(compound)
,	m_attributes(&attributes)
{
}

MemberComplex::~MemberComplex()
{
}

}

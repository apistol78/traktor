/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/MemberArray.h"

namespace traktor
{

MemberArray::MemberArray(const wchar_t* const name, const Attribute* attributes)
:	m_name(name)
,	m_attributes(attributes)
{
}

void MemberArray::setAttributes(const Attribute* attributes)
{
    m_attributes = attributes;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeDirection.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeDirection", AttributeDirection, Attribute)

AttributeDirection::AttributeDirection(bool unit)
:	m_unit(unit)
{
}

Ref< Attribute > AttributeDirection::internalClone() const
{
	return new AttributeDirection(m_unit);
}

}

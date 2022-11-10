/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeUnit.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeUnit", AttributeUnit, Attribute)

AttributeUnit::AttributeUnit(UnitType unit, bool perSecond)
:	m_unit(unit)
,	m_perSecond(perSecond)
{
}

UnitType AttributeUnit::getUnit() const
{
	return m_unit;
}

bool AttributeUnit::getPerSecond() const
{
	return m_perSecond;
}

}

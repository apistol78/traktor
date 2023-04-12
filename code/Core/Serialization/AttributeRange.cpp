/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeRange", AttributeRange, Attribute)

AttributeRange::AttributeRange(float min, float max)
:	m_min(min)
,	m_max(max)
{
}

float AttributeRange::getMin() const
{
	return m_min;
}

float AttributeRange::getMax() const
{
	return m_max;
}

Ref< Attribute > AttributeRange::internalClone() const
{
	return new AttributeRange(m_min, m_max);
}

}

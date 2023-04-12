/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrecision.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributePrecision", AttributePrecision, Attribute)

AttributePrecision::AttributePrecision(PrecisionType precision)
:	m_precision(precision)
{
}

AttributePrecision::PrecisionType AttributePrecision::getPrecision() const
{
	return m_precision;
}

Ref< Attribute > AttributePrecision::internalClone() const
{
	return new AttributePrecision(m_precision);
}

}

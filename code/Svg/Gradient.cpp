/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/Gradient.h"

namespace traktor::svg
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Gradient", Gradient, Object)

Gradient::Gradient(GradientType gradientType)
:	m_gradientType(gradientType)
{
}

Gradient::GradientType Gradient::getGradientType() const
{
	return m_gradientType;
}

void Gradient::addStop(float offset, const Color4f& color)
{
	m_stops.push_back({ offset, color });
}

const AlignedVector< Gradient::Stop >& Gradient::getStops() const
{
	return m_stops;
}

}

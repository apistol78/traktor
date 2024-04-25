/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
,	m_transform(Matrix33::identity())
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

uint32_t Gradient::getStopCount() const
{
	return (uint32_t)m_stops.size();
}

float Gradient::getStopOffset(uint32_t index) const
{
	return m_stops[index].offset;
}

Color4f Gradient::getStopColor(uint32_t index) const
{
	return m_stops[index].color;
}

void Gradient::setStops(const AlignedVector< Stop >& stops)
{
	m_stops = stops;
}

const AlignedVector< Gradient::Stop >& Gradient::getStops() const
{
	return m_stops;
}

void Gradient::setBounds(const Aabb2& bounds)
{
	m_bounds = bounds;
}

const Aabb2& Gradient::getBounds() const
{
	return m_bounds;
}

void Gradient::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& Gradient::getTransform() const
{
	return m_transform;
}

}

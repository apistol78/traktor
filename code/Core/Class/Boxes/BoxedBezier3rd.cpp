/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/Boxes/BoxedBezier2nd.h"
#include "Core/Class/Boxes/BoxedBezier3rd.h"

namespace traktor
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Bezier3rd", BoxedBezier3rd, Boxed)

BoxedBezier3rd::BoxedBezier3rd(const Bezier3rd& value)
:	m_value(value)
{
}

BoxedBezier3rd::BoxedBezier3rd(const BoxedVector2* cp0, const BoxedVector2* cp1, const BoxedVector2* cp2, const BoxedVector2* cp3)
:	m_value(cp0->unbox(), cp1->unbox(), cp2->unbox(), cp3->unbox())
{
}

std::wstring BoxedBezier3rd::toString() const
{
	return L"(Bezier3rd)";
}

Vector2 BoxedBezier3rd::evaluate(float t) const
{
	return m_value.evaluate(t);
}

Vector2 BoxedBezier3rd::tangent(float t) const
{
	return m_value.tangent(t);
}

float BoxedBezier3rd::flatness() const
{
	return m_value.flatness();
}

bool BoxedBezier3rd::isFlat(float tolerance) const
{
	return m_value.isFlat(tolerance);
}

RefArray< BoxedBezier3rd > BoxedBezier3rd::split(float t) const
{
	Bezier3rd left, right;
	m_value.split(t, left, right);

	RefArray< BoxedBezier3rd > out(2);
	out[0] = new BoxedBezier3rd(left);
	out[1] = new BoxedBezier3rd(right);
	return out;
}

RefArray< BoxedBezier2nd > BoxedBezier3rd::approximate(float errorThreshold, int maxSubdivisions) const
{
	AlignedVector< Bezier2nd > quadratic;
	m_value.approximate(errorThreshold, maxSubdivisions, quadratic);

	RefArray< BoxedBezier2nd > out(quadratic.size());
	for (size_t i = 0; i < quadratic.size(); ++i)
		out[i] = new BoxedBezier2nd(quadratic[i]);
	return out;
}

}

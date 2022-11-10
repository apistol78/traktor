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
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Bezier2nd", BoxedBezier2nd, Boxed)

BoxedBezier2nd::BoxedBezier2nd(const Bezier2nd& value)
:	m_value(value)
{
}

BoxedBezier2nd::BoxedBezier2nd(const BoxedVector2* cp0, const BoxedVector2* cp1, const BoxedVector2* cp2)
:	m_value(cp0->unbox(), cp1->unbox(), cp2->unbox())
{
}

std::wstring BoxedBezier2nd::toString() const
{
	return L"(Bezier2nd)";
}

Vector2 BoxedBezier2nd::evaluate(float t) const
{
	return m_value.evaluate(t);
}

Vector2 BoxedBezier2nd::tangent(float t) const
{
	return m_value.tangent(t);
}

float BoxedBezier2nd::getLocalMinMaxY() const
{
	return m_value.getLocalMinMaxY();
}

float BoxedBezier2nd::getLocalMinMaxX() const
{
	return m_value.getLocalMinMaxX();
}

Range< float > BoxedBezier2nd::intersectX(float y) const
{
	float t0, t1;
	m_value.intersectX(y, t0, t1);
	return Range< float >(t0, t1);
}

Range< float > BoxedBezier2nd::intersectY(float x) const
{
	float t0, t1;
	m_value.intersectY(x, t0, t1);
	return Range< float >(t0, t1);
}

RefArray< BoxedBezier2nd > BoxedBezier2nd::split(float t) const
{
	Bezier2nd left, right;
	m_value.split(t, left, right);

	RefArray< BoxedBezier2nd > out(2);
	out[0] = new BoxedBezier2nd(left);
	out[1] = new BoxedBezier2nd(right);
	return out;
}

Ref< BoxedBezier3rd > BoxedBezier2nd::toBezier3rd() const
{
	Bezier3rd out;
	m_value.toBezier3rd(out);
	return new BoxedBezier3rd(out);
}

}

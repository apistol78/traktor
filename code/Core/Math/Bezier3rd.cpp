/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"

namespace traktor
{

Bezier3rd::Bezier3rd(const Vector2& cp0_, const Vector2& cp1_, const Vector2& cp2_, const Vector2& cp3_)
:	cp0(cp0_)
,	cp1(cp1_)
,	cp2(cp2_)
,	cp3(cp3_)
{
}

Vector2 Bezier3rd::evaluate(float t) const
{
	const Vector2 D = cp0;
	const Vector2 C = 3.0f * cp1 - 3.0f * cp0;
	const Vector2 B = 3.0f * cp2 - 6.0f * cp1 + 3.0f * cp0;
	const Vector2 A = cp3 - 3.0f * cp2 + 3.0f * cp1 - cp0;
	return (t * t * t) * A + (t * t) * B + t * C + D;
}

Vector2 Bezier3rd::tangent(float t) const
{
	const Vector2 C = 3.0f * cp1 - 3.0f * cp0;
	const Vector2 B = 3.0f * cp2 - 6.0f * cp1 + 3.0f * cp0;
	const Vector2 A = cp3 - 3.0f * cp2 + 3.0f * cp1 - cp0;
	return (3.0f * t * t) * A + (2.0f * t) * B + C;
}

float Bezier3rd::flatness() const
{
	Vector2 u = 3.0f * cp1 - 2.0f * cp0 - cp3; u *= u;
	Vector2 v = 3.0f * cp2 - 2.0f * cp3 - cp0; v *= v;

	if (u.x < v.x)
		u.x = v.x;
	if (u.y < v.y)
		u.y = v.y;

	return u.x + u.y;
}

bool Bezier3rd::isFlat(float tolerance) const
{
	return flatness() <= 16.0f * tolerance * tolerance;
}

void Bezier3rd::split(float t, Bezier3rd& outLeft, Bezier3rd& outRight) const
{
	const Vector2 p = evaluate(t);
	const Vector2 c_12 = lerp(cp0, cp1, t);
	const Vector2 c_23 = lerp(cp1, cp2, t);
	const Vector2 c_34 = lerp(cp2, cp3, t);
	const Vector2 c_123 = lerp(c_12, c_23, t);
	const Vector2 c_234 = lerp(c_23, c_34, t);
	outLeft = Bezier3rd(
		cp0,
		c_12,
		c_123,
		p
	);
	outRight = Bezier3rd(
		p,
		c_234,
		c_34,
		cp3
	);
}

namespace
{

	void approximateSubdivide(const Bezier3rd& b, float errorThreshold, int maxSubdivisions, AlignedVector< Bezier2nd >& outQuadratic)
	{
		if (maxSubdivisions <= 1)
		{
			const Vector2 p_m = b.evaluate(0.5f);
			outQuadratic.push_back(Bezier2nd::fromPoints(b.cp0, p_m, b.cp3));
			return;
		}

		Bezier3rd bl, br;
		b.split(0.5f, bl, br);

		if (bl.isFlat(errorThreshold))
		{
			const Vector2 p_m = bl.evaluate(0.5f);
			outQuadratic.push_back(Bezier2nd::fromPoints(bl.cp0, p_m, bl.cp3));
		}
		else
		{
			approximateSubdivide(bl, errorThreshold, maxSubdivisions - 1, outQuadratic);
		}

		if (br.isFlat(errorThreshold))
		{
			const Vector2 p_m = br.evaluate(0.5f);
			outQuadratic.push_back(Bezier2nd::fromPoints(br.cp0, p_m, br.cp3));
		}
		else
		{
			approximateSubdivide(br, errorThreshold, maxSubdivisions - 1, outQuadratic);
		}
	}

}

void Bezier3rd::approximate(float errorThreshold, int maxSubdivisions, AlignedVector< Bezier2nd >& outQuadratic) const
{
	outQuadratic.resize(0);
	approximateSubdivide(*this, errorThreshold, maxSubdivisions, outQuadratic);
}

Bezier3rd Bezier3rd::fromCatmullRom(const Vector2& cp0, const Vector2& cp1, const Vector2& cp2, const Vector2& cp3, float tension)
{
	const Vector2 bcp0 = cp1;
	const Vector2 bcp1 = cp1 + (cp2 - cp0) / (6.0f * tension);
	const Vector2 bcp2 = cp2 - (cp3 - cp1) / (6.0f * tension);
	const Vector2 bcp3 = cp2;
	return Bezier3rd(bcp0, bcp1, bcp2, bcp3);
}

}

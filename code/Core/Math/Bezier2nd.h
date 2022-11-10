/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Bezier3rd;

/*! Quadratic bezier curve.
 * \ingroup Core
 */
class T_DLLCLASS Bezier2nd
{
public:
	Vector2 cp0;
	Vector2 cp1;
	Vector2 cp2;

	Bezier2nd() = default;

	Bezier2nd(const Vector2& cp0, const Vector2& cp1, const Vector2& cp2);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	float getLocalMinMaxY() const;

	float getLocalMinMaxX() const;

	void intersectX(float y, float& outT0, float& outT1) const;

	void intersectY(float x, float& outT0, float& outT1) const;

	void split(float t, Bezier2nd& outLeft, Bezier2nd& outRight) const;

	void toBezier3rd(Bezier3rd& out3rd) const;

	/*! Construct a 2nd order bezier curve from three points
	 * by which all three lies on the curve.
	 */
	static Bezier2nd fromPoints(const Vector2& p0, const Vector2& p1, const Vector2& p2);
};

}


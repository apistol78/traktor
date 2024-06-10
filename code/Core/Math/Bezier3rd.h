/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
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

class Bezier2nd;

/*! Cubic bezier curve.
 * \ingroup Core
 */
class T_DLLCLASS Bezier3rd
{
public:
	Vector2 cp0;
	Vector2 cp1;
	Vector2 cp2;
	Vector2 cp3;

	Bezier3rd() = default;

	Bezier3rd(const Vector2& cp0, const Vector2& cp1, const Vector2& cp2, const Vector2& cp3);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	float flatness() const;

	/*! Check if curve is sufficiently flat.
	 *
	 * \param tolerance Tolerance value as defined by PostScript.
	 */
	bool isFlat(float tolerance) const;

	void split(float t, Bezier3rd& outLeft, Bezier3rd& outRight) const;

	void approximate(float errorThreshold, int maxSubdivisions, AlignedVector< Bezier2nd >& outQuadratic) const;

	static Bezier3rd fromCatmullRom(const Vector2& cp0, const Vector2& cp1, const Vector2& cp2, const Vector2& cp3, float tension);
};

}


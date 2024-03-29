/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Ray2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! 2D line
 * \ingroup Core
 */
class T_DLLCLASS Line2
{
public:
	Vector2 p[2];

	T_MATH_INLINE Line2() = default;

	T_MATH_INLINE Line2(const Vector2& p0, const Vector2& p1);

	T_MATH_INLINE Vector2 delta() const;

	T_MATH_INLINE float length() const;

	T_MATH_INLINE Vector2 normal() const;

	T_MATH_INLINE Vector2 center() const;

	T_MATH_INLINE float distance(const Vector2& pt) const;

	T_MATH_INLINE Vector2 project(const Vector2& pt) const;

	T_MATH_INLINE bool inrange(const Vector2& pt) const;

	T_MATH_INLINE bool classify(const Vector2& pt, float thickness) const;

	/*! Intersect line segment with ray.
	 *
	 * \param ray Intersecting ray.
	 * \param outR Fraction on line segment; Pi = P0 + Pd * outR
	 * \param outK Distance of ray; Pi = origin + direction * outK
	 */
	T_MATH_INLINE bool intersect(const Ray2& ray, float& outR, float& outK) const;

	T_MATH_INLINE Line2 operator + (const Vector2& v) const;

	T_MATH_INLINE Line2 operator - (const Vector2& v) const;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Line2.inl"
#endif


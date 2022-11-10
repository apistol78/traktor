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

/*! 2D ray
 * \ingroup Core
 */
class T_DLLCLASS Ray2
{
public:
	Vector2 origin;
	Vector2 direction;

	T_MATH_INLINE Ray2();

	T_MATH_INLINE Ray2(const Vector2& origin, const Vector2& direction);

	T_MATH_INLINE Vector2 normal() const;

	T_MATH_INLINE float distance(const Vector2& pt) const;

	/*! Calculate unique intersection between two rays.
	 *
	 * \param ray Intersecting ray.
	 * \param outR Distance on "this" ray.
	 * \param outK Distance on "other" ray.
	 */
	T_MATH_INLINE bool intersect(const Ray2& ray, float& outR, float& outK) const;

	T_MATH_INLINE Vector2 operator * (float k) const;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Ray2.inl"
#endif


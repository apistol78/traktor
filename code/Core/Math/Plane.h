/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix44;

/*! 3d plane.
 * \ingroup Core
 *
 * Ax + By + Cz + D = 0
 * where
 * [ A B C ] == normal
 */
class T_DLLCLASS Plane
{
public:
	T_MATH_INLINE Plane();

	T_MATH_INLINE Plane(const Plane& src);

	/*! Construct plane from normal and distance. */
	explicit T_MATH_INLINE Plane(const Vector4& normal, const Scalar& distance);

	/*! Construct plane from normal and a point in the specified plane. */
	explicit T_MATH_INLINE Plane(const Vector4& normal, const Vector4& pointInPlane);

	/*! Construct plane from three points in the specified plane. */
	explicit T_MATH_INLINE Plane(const Vector4& a, const Vector4& b, const Vector4& c);

	/*! Construct plane from four explicit constants. */
	explicit T_MATH_INLINE Plane(float a, float b, float c, float d);

	T_MATH_INLINE void set(const Vector4& normal, const Scalar& distance);

	T_MATH_INLINE void set(float a, float b, float c, float d);

	T_MATH_INLINE void setNormal(const Vector4& normal);

	T_MATH_INLINE void setDistance(const Scalar& distance);

	T_MATH_INLINE Vector4 normal() const;

	T_MATH_INLINE Scalar distance() const;

	T_MATH_INLINE Scalar distance(const Vector4& point) const;

	T_MATH_INLINE Vector4 project(const Vector4& v) const;

	T_MATH_INLINE bool intersectRay(
		const Vector4& origin,
		const Vector4& direction,
		Scalar& outK
	) const;

	T_MATH_INLINE bool intersectRay(
		const Vector4& origin,
		const Vector4& direction,
		Scalar& outK,
		Vector4& outPoint
	) const;

	T_MATH_INLINE bool intersectSegment(
		const Vector4& a,
		const Vector4& b,
		Scalar& outK,
		Vector4* outPoint = 0
	) const;

	T_MATH_INLINE static bool uniqueIntersectionPoint(
		const Plane& a,
		const Plane& b,
		const Plane& c,
		Vector4& outPoint
	);

	T_MATH_INLINE Plane& operator = (const Plane& src);

	friend T_DLLCLASS Plane operator * (const Matrix44& m, const Plane& pl);

private:
	Vector4 m_normal;
	Scalar m_distance;
};

T_MATH_INLINE T_DLLCLASS Plane operator * (const Matrix44& m, const Plane& pl);

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Plane.inl"
#endif


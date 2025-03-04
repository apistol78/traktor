/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cmath>
#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/MathUtils.h"

#if defined(min)
#	undef min
#endif
#if defined(max)
#	undef max
#endif

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! 2d vector.
 * \ingroup Core
 */
class T_DLLCLASS Vector2
{
public:
#if defined (_MSC_VER)
#	pragma warning( disable:4201 )
#endif
	union
	{
		float e[2];
		struct { float x, y; };
	};
#if defined (_MSC_VER)
#	pragma warning( default:4201 )
#endif

	T_MATH_INLINE Vector2();

	T_MATH_INLINE Vector2(const Vector2& v);

	explicit T_MATH_INLINE Vector2(float x_, float y_);

	static T_MATH_INLINE const Vector2& zero();

	static T_MATH_INLINE const Vector2& one();

	T_MATH_INLINE void set(float x_, float y_);

	template < int iX, int iY >
	T_MATH_INLINE Vector2 shuffle() const
	{
		return Vector2(e[iX], e[iY]);
	}

	T_MATH_INLINE float length() const;

	T_MATH_INLINE float length2() const;

	T_MATH_INLINE Vector2 absolute() const;

	T_MATH_INLINE Vector2 normalized() const;

	T_MATH_INLINE Vector2 perpendicular() const;

	T_MATH_INLINE Vector2& operator = (const Vector2& v);

	T_MATH_INLINE Vector2 operator - () const;

	T_MATH_INLINE Vector2& operator += (float scalar);

	T_MATH_INLINE Vector2& operator += (const Vector2& v);

	T_MATH_INLINE Vector2& operator -= (float scalar);

	T_MATH_INLINE Vector2& operator -= (const Vector2& v);

	T_MATH_INLINE Vector2& operator *= (float scalar);

	T_MATH_INLINE Vector2& operator *= (const Vector2& v);

	T_MATH_INLINE Vector2& operator /= (float scalar);

	T_MATH_INLINE Vector2& operator /= (const Vector2& v);

	T_MATH_INLINE bool operator == (const Vector2& v) const;

	T_MATH_INLINE bool operator != (const Vector2& v) const;

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator + (const Vector2& l, float r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator + (float l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator + (const Vector2& l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator - (const Vector2& l, float r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator - (float l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator - (const Vector2& l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator * (const Vector2& l, float r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator * (float l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator * (const Vector2& l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator / (const Vector2& l, float r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator / (float l, const Vector2& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator / (const Vector2& l, const Vector2& r);
};

T_MATH_INLINE T_DLLCLASS float dot(const Vector2& l, const Vector2& r);

T_MATH_INLINE T_DLLCLASS Vector2 lerp(const Vector2& a, const Vector2& b, float c);

T_MATH_INLINE T_DLLCLASS Vector2 lerp(const Vector2& a, const Vector2& b, const Vector2& c);

T_MATH_INLINE T_DLLCLASS float determinant(const Vector2& a, const Vector2& b);

T_MATH_INLINE T_DLLCLASS Vector2 min(const Vector2& l, const Vector2& r);

T_MATH_INLINE T_DLLCLASS Vector2 max(const Vector2& l, const Vector2& r);

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Vector2.inl"
#endif


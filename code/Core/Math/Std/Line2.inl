/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{

T_MATH_INLINE Line2::Line2()
{
}

T_MATH_INLINE Line2::Line2(const Vector2& p0, const Vector2& p1)
{
	p[0] = p0;
	p[1] = p1;
}

T_MATH_INLINE Vector2 Line2::delta() const
{
	return p[1] - p[0];
}

T_MATH_INLINE float Line2::length() const
{
	return delta().length();
}

T_MATH_INLINE Vector2 Line2::normal() const
{
	return delta().perpendicular().normalized();
}

T_MATH_INLINE Vector2 Line2::center() const
{
	return (p[0] + p[1]) / 2;
}

T_MATH_INLINE float Line2::distance(const Vector2& pt) const
{
	return dot(normal(), pt - p[0]);
}

T_MATH_INLINE Vector2 Line2::project(const Vector2& pt) const
{
	float k = distance(pt);
	return pt - normal() * k;
}

T_MATH_INLINE bool Line2::inrange(const Vector2& pt) const
{
	float k = dot(pt - p[0], delta()) / (length() * length());
	return k >= 0.0f && k <= 1.0f;
}

T_MATH_INLINE bool Line2::classify(const Vector2& pt, float thickness) const
{
	float k = distance(pt);
	if (abs(k) > thickness)
		return false;
	Vector2 p1 = pt - normal() * k;
	return inrange(p1);
}

T_MATH_INLINE bool Line2::intersect(const Ray2& ray, float& outR, float& outK) const
{
	Ray2 ray0(p[0], (p[1] - p[0]).normalized());
	if (ray0.intersect(ray, outR, outK))
		return outR >= -FUZZY_EPSILON && outR <= 1.0f + FUZZY_EPSILON;
	else
		return false;
}

T_MATH_INLINE Line2 Line2::operator + (const Vector2& v) const
{
	return Line2(
		p[0] + v,
		p[1] + v
	);
}

T_MATH_INLINE Line2 Line2::operator - (const Vector2& v) const
{
	return Line2(
		p[0] - v,
		p[1] - v
	);
}

}

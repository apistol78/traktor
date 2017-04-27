/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Plane.h"
#include "Core/Math/Matrix44.h"

namespace traktor
{

T_MATH_INLINE Plane::Plane()
{
}

T_MATH_INLINE Plane::Plane(const Plane& src)
:	m_normal(src.m_normal)
,	m_distance(src.m_distance)
{
}

T_MATH_INLINE Plane::Plane(const Vector4& normal, const Scalar& distance)
:	m_normal(normal.xyz0())
,	m_distance(distance)
{
}

T_MATH_INLINE Plane::Plane(const Vector4& normal, const Vector4& pointInPlane)
:	m_normal(normal.xyz0())
{
	m_distance = dot3(m_normal, pointInPlane);
}

T_MATH_INLINE Plane::Plane(const Vector4& a, const Vector4& b, const Vector4& c)
{
	m_normal = cross(c - a, b - a).normalized();
	m_distance = dot3(m_normal, a);
}

T_MATH_INLINE Plane::Plane(float a, float b, float c, float d)
{
	m_normal = Vector4(a, b, c, 0.0f);
	m_distance = -Scalar(d);
}

T_MATH_INLINE void Plane::set(const Vector4& normal, const Scalar& distance)
{
	m_normal = normal.xyz0();
	m_distance = distance;
}

T_MATH_INLINE void Plane::set(float a, float b, float c, float d)
{
	m_normal = Vector4(a, b, c, 0.0f);
	m_distance = -Scalar(d);
}

T_MATH_INLINE void Plane::setNormal(const Vector4& normal)
{
	m_normal = normal;
}

T_MATH_INLINE void Plane::setDistance(const Scalar& distance)
{
	m_distance = distance;
}

T_MATH_INLINE Vector4 Plane::normal() const
{
	return m_normal;
}

T_MATH_INLINE Scalar Plane::distance() const
{
	return m_distance;
}

T_MATH_INLINE Scalar Plane::distance(const Vector4& point) const
{
	return dot3(m_normal, point) - m_distance;
}

T_MATH_INLINE Vector4 Plane::project(const Vector4& v) const
{
	return v - m_normal * distance(v);
}

T_MATH_INLINE bool Plane::rayIntersection(
	const Vector4& origin,
	const Vector4& direction,
	Scalar& outK,
	Vector4* outPoint
) const
{
	Scalar denom = -dot3(m_normal, direction);
	if (denom == 0.0f)
		return false;

	Scalar divend = distance(origin);

	outK = divend / denom;
	if (outPoint != 0)
		*outPoint = origin + direction * outK;

	return true;
}

T_MATH_INLINE bool Plane::segmentIntersection(
	const Vector4& a,
	const Vector4& b,
	Scalar& outK,
	Vector4* outPoint
) const
{
	Vector4 d = b - a;
	
	Scalar denom = dot3(m_normal, d);
	if (denom == 0.0f)
		return false;

	Scalar divend = -dot3(m_normal, a) + m_distance;

	outK = divend / denom;
	if (outK < 0.0f || outK > 1.0f)
		return false;

	if (outPoint != 0)
		*outPoint = a + d * outK;

	return true;
}

T_MATH_INLINE bool Plane::uniqueIntersectionPoint(
	const Plane& a,
	const Plane& b,
	const Plane& c,
	Vector4& outPoint
)
{
	// n1 * (n2 x n3) != 0
	Scalar denom = dot3(a.m_normal, cross(b.m_normal, c.m_normal));
	if (denom == 0.0f)
		return false;

	// d1(n2 x n3) + d2(n3 x n1) + d3(n1 x n2)
	Vector4 divend =
		a.m_distance * cross(b.m_normal, c.m_normal) +
		b.m_distance * cross(c.m_normal, a.m_normal) +
		c.m_distance * cross(a.m_normal, b.m_normal);

	outPoint = (divend / denom).xyz1();
	return true;
}

T_MATH_INLINE Plane& Plane::operator = (const Plane& src)
{
	m_normal = src.m_normal;
	m_distance = src.m_distance;
	return *this;
}

T_MATH_INLINE Plane operator * (const Matrix44& m, const Plane& pl)
{
	Vector4 N = m * pl.normal().xyz0();
	Vector4 P = m * (pl.normal().xyz0() * pl.distance()).xyz1();
	return Plane(N, P);
}

}

#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"

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

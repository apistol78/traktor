#include <cmath>
#include "Core/Math/Vector2i.h"

namespace traktor
{

T_MATH_INLINE Vector2i::Vector2i()
{
}

T_MATH_INLINE Vector2i::Vector2i(const Vector2i& v)
:	x(v.x)
,	y(v.y)
{
}

T_MATH_INLINE Vector2i::Vector2i(int32_t x_, int32_t y_)
:	x(x_)
,	y(y_)
{
}

T_MATH_INLINE Vector2 Vector2i::toVector2() const
{
	return Vector2(float(x), float(y));
}

T_MATH_INLINE Vector2i Vector2i::fromVector2(const Vector2& v)
{
	return Vector2i(int32_t(std::floor(v.x)), int32_t(std::floor(v.y)));
}

T_MATH_INLINE bool Vector2i::operator == (const Vector2i& r) const
{
	return x == r.x && y == r.y;
}

T_MATH_INLINE bool Vector2i::operator != (const Vector2i& r) const
{
	return x != r.x || y != r.y;
}

T_MATH_INLINE Vector2i operator + (const Vector2i& l, const Vector2i& r)
{
	return Vector2i(l.x + r.x, l.x + r.x);
}

T_MATH_INLINE Vector2i operator - (const Vector2i& l, const Vector2i& r)
{
	return Vector2i(l.x - r.x, l.x - r.x);
}

}

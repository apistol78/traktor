/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
namespace traktor
{

T_MATH_INLINE Vector2i::Vector2i()
{
	x =
	y = 0;
}

T_MATH_INLINE Vector2i::Vector2i(const Vector2i& v)
{
	x = v.x;
	y = v.y;
}

T_MATH_INLINE Vector2i::Vector2i(int32_t x_, int32_t y_)
{
	this->x = x_;
	this->y = y_;
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

T_MATH_INLINE Vector2i operator * (const Vector2i& l, int32_t r)
{
	return Vector2i(l.x * r, l.y * r);
}

T_MATH_INLINE Vector2i operator * (int32_t l, const Vector2i& r)
{
	return Vector2i(r.x * l, r.y * l);
}

T_MATH_INLINE Vector2i operator / (const Vector2i& l, int32_t r)
{
	return Vector2i(l.x / r, l.y / r);
}

T_MATH_INLINE Vector2i operator / (int32_t l, const Vector2i& r)
{
	return Vector2i(l / r.x, l / r.y);
}

}

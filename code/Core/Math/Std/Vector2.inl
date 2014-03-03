#include <cmath>
#include "Core/Math/Vector2.h"

namespace traktor
{

T_MATH_INLINE Vector2::Vector2()
{
	x =
	y = 0;
}

T_MATH_INLINE Vector2::Vector2(const Vector2& v)
{
	x = v.x;
	y = v.y;
}

T_MATH_INLINE Vector2::Vector2(float x, float y)
{
	this->x = x;
	this->y = y;
}

T_MATH_INLINE const Vector2& Vector2::zero()
{
	static const Vector2 c_zero(0.0f, 0.0f);
	return c_zero;
}

T_MATH_INLINE const Vector2& Vector2::one()
{
	static const Vector2 c_one(1.0f, 1.0f);
	return c_one;
}

T_MATH_INLINE void Vector2::set(float x, float y)
{
	this->x = x;
	this->y = y;
}

T_MATH_INLINE float Vector2::length() const
{
	return float(std::sqrt(length2()));
}

T_MATH_INLINE float Vector2::length2() const
{
	return dot(*this, *this);
}

T_MATH_INLINE Vector2 Vector2::normalized() const
{
	return *this / length();
}

T_MATH_INLINE Vector2 Vector2::perpendicular() const
{
	return Vector2(y, -x);
}

T_MATH_INLINE Vector2& Vector2::operator = (const Vector2& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

T_MATH_INLINE Vector2 Vector2::operator - () const
{
	return Vector2(-x, -y);
}

T_MATH_INLINE Vector2& Vector2::operator += (float scalar)
{
	x += scalar;
	y += scalar;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator += (const Vector2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator -= (float scalar)
{
	x -= scalar;
	y -= scalar;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator -= (const Vector2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator *= (float scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator *= (const Vector2& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator /= (float scalar)
{
	x /= scalar;
	y /= scalar;
	return *this;
}

T_MATH_INLINE Vector2& Vector2::operator /= (const Vector2& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

T_MATH_INLINE bool Vector2::operator == (const Vector2& v) const
{
	return bool(x == v.x && y == v.y);
}

T_MATH_INLINE bool Vector2::operator != (const Vector2& v) const
{
	return bool(x != v.x || y != v.y);
}

T_MATH_INLINE Vector2 operator + (const Vector2& l, float r)
{
	return Vector2(l.x + r, l.y + r);
}

T_MATH_INLINE Vector2 operator + (float l, const Vector2& r)
{
	return Vector2(l + r.x, l + r.y);
}

T_MATH_INLINE Vector2 operator + (const Vector2& l, const Vector2& r)
{
	return Vector2(l.x + r.x, l.y + r.y);
}

T_MATH_INLINE Vector2 operator - (const Vector2& l, float r)
{
	return Vector2(l.x - r, l.y - r);
}

T_MATH_INLINE Vector2 operator - (float l, const Vector2& r)
{
	return Vector2(l - r.x, l - r.y);
}

T_MATH_INLINE Vector2 operator - (const Vector2& l, const Vector2& r)
{
	return Vector2(l.x - r.x, l.y - r.y);
}

T_MATH_INLINE Vector2 operator * (const Vector2& l, float r)
{
	return Vector2(l.x * r, l.y * r);
}

T_MATH_INLINE Vector2 operator * (float l, const Vector2& r)
{
	return Vector2(l * r.x, l * r.y);
}

T_MATH_INLINE Vector2 operator * (const Vector2& l, const Vector2& r)
{
	return Vector2(l.x * r.x, l.y * r.y);
}

T_MATH_INLINE Vector2 operator / (const Vector2& l, float r)
{
	return Vector2(l.x / r, l.y / r);
}

T_MATH_INLINE Vector2 operator / (float l, const Vector2& r)
{
	return Vector2(l / r.x, l / r.y);
}

T_MATH_INLINE Vector2 operator / (const Vector2& l, const Vector2& r)
{
	return Vector2(l.x / r.x, l.y / r.y);
}

T_MATH_INLINE float dot(const Vector2& l, const Vector2& r)
{
	return l.x * r.x + l.y * r.y;
}

T_MATH_INLINE Vector2 lerp(const Vector2& a, const Vector2& b, float c)
{
	return a + (b - a) * c;
}

T_MATH_INLINE float determinant(const Vector2& a, const Vector2& b)
{
	return a.x * b.y - a.y * b.x;
}

T_MATH_INLINE Vector2 min(const Vector2& l, const Vector2& r)
{
	return Vector2(
		std::min(l.x, r.x),
		std::min(l.y, r.y)
	);
}

T_MATH_INLINE Vector2 max(const Vector2& l, const Vector2& r)
{
	return Vector2(
		std::max(l.x, r.x),
		std::max(l.y, r.y)
	);
}

T_MATH_INLINE Vector2 reflect(const Vector2& v, const Vector2& at)
{
	Vector2 N = at.normalized();
	Vector2 V = N * (dot(N, v) * 2.0f);
	return V - v;
}

}

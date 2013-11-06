#include <cmath>
#include <limits>
#include "Core/Math/Const.h"
#if !defined(NDEBUG)
#	include "Core/Math/Float.h"
#endif
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace
	{

static const Vector4 c_zero(0.0f, 0.0f, 0.0f, 0.0f);
static const Vector4 c_one(1.0f, 1.0f, 1.0f, 1.0f);
static const Vector4 c_origo(0.0f, 0.0f, 0.0f, 1.0f);

inline bool compare(float e1, float e2)
{
	return abs(e2 - e1) <= 1e-3f;
}

	}

//#if !defined(NDEBUG)
//#   define VALIDATE(v) \
//	T_ASSERT (!isNan((v)._x) && !isInfinite((v)._x)); \
//	T_ASSERT (!isNan((v)._y) && !isInfinite((v)._y)); \
//	T_ASSERT (!isNan((v)._z) && !isInfinite((v)._z)); \
//	T_ASSERT (!isNan((v)._w) && !isInfinite((v)._w));
//#else
#   define VALIDATE(v)
//#endif

T_MATH_INLINE Vector4::Vector4()
#if defined(_DEBUG)
:	_x(std::numeric_limits< float >::signaling_NaN())
,	_y(std::numeric_limits< float >::signaling_NaN())
,	_z(std::numeric_limits< float >::signaling_NaN())
,	_w(std::numeric_limits< float >::signaling_NaN())
#endif
{
}

T_MATH_INLINE Vector4::Vector4(const Vector4& v)
:	_x(v._x)
,	_y(v._y)
,	_z(v._z)
,	_w(v._w)
{
	VALIDATE(*this);
}

T_MATH_INLINE Vector4::Vector4(const Scalar& s)
:	_x(s.m_data)
,	_y(s.m_data)
,	_z(s.m_data)
,	_w(s.m_data)
{
}

T_MATH_INLINE Vector4::Vector4(float x, float y, float z, float w)
:	_x(x)
,	_y(y)
,	_z(z)
,	_w(w)
{
	VALIDATE(*this);
}

T_MATH_INLINE Vector4::Vector4(const float* p)
{
	T_ASSERT (p);
	_x = p[0];
	_y = p[1];
	_z = p[2];
	_w = p[3];
	VALIDATE(*this);
}

T_MATH_INLINE const Vector4& Vector4::zero()
{
	return c_zero;
}

T_MATH_INLINE const Vector4& Vector4::one()
{
	return c_one;
}

T_MATH_INLINE const Vector4& Vector4::origo()
{
	return c_origo;
}

T_MATH_INLINE void Vector4::set(float x, float y, float z, float w)
{
	_x = x;
	_y = y;
	_z = z;
	_w = w;
	VALIDATE(*this);
}

T_MATH_INLINE Scalar Vector4::min() const
{
	return Scalar(traktor::min(traktor::min(_x, _y), traktor::min(_z, _w)));
}

T_MATH_INLINE Scalar Vector4::max() const
{
	return Scalar(traktor::max(traktor::max(_x, _y), traktor::max(_z, _w)));
}

T_MATH_INLINE Scalar Vector4::x() const
{
	return Scalar(_x);
}

T_MATH_INLINE Scalar Vector4::y() const
{
	return Scalar(_y);
}

T_MATH_INLINE Scalar Vector4::z() const
{
	return Scalar(_z);
}

T_MATH_INLINE Scalar Vector4::w() const
{
	return Scalar(_w);
}

T_MATH_INLINE Vector4 Vector4::xyz0() const
{
	return Vector4(_x, _y, _z, 0.0f);
}

T_MATH_INLINE Vector4 Vector4::xyz1() const
{
	return Vector4(_x, _y, _z, 1.0f);
}

T_MATH_INLINE Scalar Vector4::length() const
{
	VALIDATE(*this);
	return Scalar(float(std::sqrt(length2())));
}

T_MATH_INLINE Scalar Vector4::length2() const
{
	VALIDATE(*this);
	return dot4(*this, *this);
}

T_MATH_INLINE Scalar Vector4::normalize()
{
	VALIDATE(*this);
	Scalar ln = length();
	*this /= ln;
	return ln;
}

T_MATH_INLINE Vector4 Vector4::normalized() const
{
	VALIDATE(*this);
	Scalar ln = length();
	return *this / ln;
}

T_MATH_INLINE Vector4 Vector4::absolute() const
{
	VALIDATE(*this);
	return Vector4(
		abs(_x),
		abs(_y),
		abs(_z),
		abs(_w)
	);
}

T_MATH_INLINE Vector4 Vector4::loadAligned(const float* in)
{
	T_ASSERT (in);
	return Vector4(in);
}

T_MATH_INLINE Vector4 Vector4::loadUnaligned(const float* in)
{
	T_ASSERT (in);
	return Vector4(in);
}

T_MATH_INLINE void Vector4::storeAligned(float* out) const
{
	T_ASSERT (out);
	out[0] = _x;
	out[1] = _y;
	out[2] = _z;
	out[3] = _w;
}

T_MATH_INLINE void Vector4::storeUnaligned(float* out) const
{
	T_ASSERT (out);
	out[0] = _x;
	out[1] = _y;
	out[2] = _z;
	out[3] = _w;
}

T_MATH_INLINE Scalar Vector4::get(int index) const
{
	switch (index)
	{
	case 0:
		return Scalar(_x);
	case 1:
		return Scalar(_y);
	case 2:
		return Scalar(_z);
	case 3:
		return Scalar(_w);
	}
	return Scalar();
}

T_MATH_INLINE void Vector4::set(int index, const Scalar& value)
{
	switch (index)
	{
	case 0:
		_x = value.m_data;
		break;
	case 1:
		_y = value.m_data;
		break;
	case 2:
		_z = value.m_data;
		break;
	case 3:
		_w = value.m_data;
		break;
	}
}

T_MATH_INLINE Vector4& Vector4::operator = (const Vector4& v)
{
	_x = v._x;
	_y = v._y;
	_z = v._z;
	_w = v._w;
	return *this;
}

T_MATH_INLINE Vector4 Vector4::operator - () const
{
	VALIDATE(*this);
	return Vector4(-_x, -_y, -_z, -_w);
}

T_MATH_INLINE Vector4& Vector4::operator += (const Scalar& v)
{
	_x += v.m_data;
	_y += v.m_data;
	_z += v.m_data;
	_w += v.m_data;
	VALIDATE(*this);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator += (const Vector4& v)
{
	VALIDATE(v);
	_x += v._x;
	_y += v._y;
	_z += v._z;
	_w += v._w;
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Scalar& v)
{
	_x -= v.m_data;
	_y -= v.m_data;
	_z -= v.m_data;
	_w -= v.m_data;
	VALIDATE(*this);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Vector4& v)
{
	VALIDATE(v);
	_x -= v._x;
	_y -= v._y;
	_z -= v._z;
	_w -= v._w;
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Scalar& v)
{
	_x *= v.m_data;
	_y *= v.m_data;
	_z *= v.m_data;
	_w *= v.m_data;
	VALIDATE(*this);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Vector4& v)
{
	VALIDATE(v);
	_x *= v._x;
	_y *= v._y;
	_z *= v._z;
	_w *= v._w;
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Scalar& v)
{
	_x /= v.m_data;
	_y /= v.m_data;
	_z /= v.m_data;
	_w /= v.m_data;
	VALIDATE(*this);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Vector4& v)
{
	VALIDATE(v);
	_x /= v._x;
	_y /= v._y;
	_z /= v._z;
	_w /= v._w;
	return *this;
}

T_MATH_INLINE bool Vector4::operator == (const Vector4& v) const
{
	VALIDATE(v);
	return bool(
		compare(_x, v._x) &&
		compare(_y, v._y) &&
		compare(_z, v._z) &&
		compare(_w, v._w)
	);
}

T_MATH_INLINE bool Vector4::operator != (const Vector4& v) const
{
	VALIDATE(v);
	return bool(
		!compare(_x, v._x) ||
		!compare(_y, v._y) ||
		!compare(_z, v._z) ||
		!compare(_w, v._w)
	);
}

T_MATH_INLINE Scalar Vector4::operator [] (int index) const
{
	switch (index)
	{
	case 0:
		return x();
	case 1:
		return y();
	case 2:
		return z();
	case 3:
		return w();
	}
	return Scalar();
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Scalar& r)
{
	VALIDATE(l);
	return Vector4(l._x + r.m_data, l._y + r.m_data, l._z + r.m_data, l._w + r.m_data);
}

T_MATH_INLINE Vector4 operator + (const Scalar& l, const Vector4& r)
{
	VALIDATE(r);
	return Vector4(l.m_data + r._x, l.m_data + r._y, l.m_data + r._z, l.m_data + r._w);
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	return Vector4(l._x + r._x, l._y + r._y, l._z + r._z, l._w + r._w);
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Scalar& r)
{
	VALIDATE(l);
	return Vector4(l._x - r.m_data, l._y - r.m_data, l._z - r.m_data, l._w - r.m_data);
}

T_MATH_INLINE Vector4 operator - (const Scalar& l, const Vector4& r)
{
	VALIDATE(r);
	return Vector4(l.m_data - r._x, l.m_data - r._y, l.m_data - r._z, l.m_data - r._w);
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Vector4(l._x - r._x, l._y - r._y, l._z - r._z, l._w - r._w);
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Scalar& r)
{
	VALIDATE(l);
	return Vector4(l._x * r.m_data, l._y * r.m_data, l._z * r.m_data, l._w * r.m_data);
}

T_MATH_INLINE Vector4 operator * (const Scalar& l, const Vector4& r)
{
	VALIDATE(r);
	return Vector4(l.m_data * r._x, l.m_data * r._y, l.m_data * r._z, l.m_data * r._w);
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Vector4(l._x * r._x, l._y * r._y, l._z * r._z, l._w * r._w);
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Scalar& r)
{
	VALIDATE(l);
	T_ASSERT(r != 0.0f);
	return Vector4(l._x / r.m_data, l._y / r.m_data, l._z / r.m_data, l._w / r.m_data);
}

T_MATH_INLINE Vector4 operator / (const Scalar& l, const Vector4& r)
{
	VALIDATE(r);
	T_ASSERT(r._x != 0.0f);
	T_ASSERT(r._y != 0.0f);
	T_ASSERT(r._z != 0.0f);
	T_ASSERT(r._w != 0.0f);
	return Vector4(l.m_data / r._x, l.m_data / r._y, l.m_data / r._z, l.m_data / r._w);
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	VALIDATE(r);
	T_ASSERT(r._x != 0.0f);
	T_ASSERT(r._y != 0.0f);
	T_ASSERT(r._z != 0.0f);
	T_ASSERT(r._w != 0.0f);
	return Vector4(l._x / r._x, l._y / r._y, l._z / r._z, l._w / r._w);
}

T_MATH_INLINE Scalar horizontalAdd3(const Vector4& v)
{
	return Scalar(v._x + v._y + v._z);
}

T_MATH_INLINE Scalar horizontalAdd4(const Vector4& v)
{
	return Scalar(v._x + v._y + v._z + v._w);
}

T_MATH_INLINE Scalar dot3(const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Scalar(l._x * r._x + l._y * r._y + l._z * r._z);
}

T_MATH_INLINE Scalar dot4(const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Scalar(l._x * r._x + l._y * r._y + l._z * r._z + l._w * r._w);
}

T_MATH_INLINE Vector4 cross(const Vector4& l, const Vector4& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Vector4(
		l._y * r._z - r._y * l._z,
		l._z * r._x - r._z * l._x,
		l._x * r._y - r._x * l._y,
		0.0f
	);
}

T_MATH_INLINE Vector4 lerp(const Vector4& a, const Vector4& b, const Scalar& c)
{
	VALIDATE(a);
	VALIDATE(b);
	return (Scalar(1.0f) - c) * a + c * b;
}

T_MATH_INLINE Vector4 reflect(const Vector4& v, const Vector4& at)
{
	VALIDATE(v);
	VALIDATE(at);
	Vector4 N = at.normalized();
	Vector4 V = N * (dot3(N, v) * Scalar(2.0f));
	return V - v;
}

T_MATH_INLINE int majorAxis3(const Vector4& v)
{
	VALIDATE(v);
	Vector4 a = v.absolute();
	return (a._x > a._y) ? ((a._x > a._z) ? 0 : 2) : ((a._y > a._z) ? 1 : 2);
}

T_MATH_INLINE Vector4 min(const Vector4& l, const Vector4& r)
{
	return Vector4(
		min< float >(l._x, r._x),
		min< float >(l._y, r._y),
		min< float >(l._z, r._z),
		min< float >(l._w, r._w)
	);
}

T_MATH_INLINE Vector4 max(const Vector4& l, const Vector4& r)
{
	return Vector4(
		max< float >(l._x, r._x),
		max< float >(l._y, r._y),
		max< float >(l._z, r._z),
		max< float >(l._w, r._w)
	);
}

T_MATH_INLINE Vector4 select(const Vector4& condition, const Vector4& negative, const Vector4& positive)
{
	return Vector4(
		condition._x < 0.0f ? negative._x : positive._x,
		condition._y < 0.0f ? negative._y : positive._y,
		condition._z < 0.0f ? negative._z : positive._z,
		condition._w < 0.0f ? negative._w : positive._w
	);		
}

T_MATH_INLINE bool compareAllGreaterEqual(const Vector4& l, const Vector4& r)
{
	return l._x >= r._x && l._y >= r._y && l._z >= r._z && l._w >= r._w;
}

T_MATH_INLINE bool compareAllLessEqual(const Vector4& l, const Vector4& r)
{
	return l._x <= r._x && l._y <= r._y && l._z <= r._z && l._w <= r._w;
}

T_MATH_INLINE bool compareFuzzyEqual(const Vector4& l, const Vector4& r)
{
	Vector4 d = (l - r).absolute();
	return d.max() <= FUZZY_EPSILON;
}

#if defined(VALIDATE)
#undef VALIDATE
#endif

}

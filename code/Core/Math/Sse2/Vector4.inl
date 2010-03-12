#include <cmath>
#include "Core/Math/Vector4.h"

namespace traktor
{

T_MATH_INLINE Vector4::Vector4()
{
}

T_MATH_INLINE Vector4::Vector4(const Vector4& v)
:	m_data(v.m_data)
{
}

T_MATH_INLINE Vector4::Vector4(__m128 v)
:	m_data(v)
{
}

T_MATH_INLINE Vector4::Vector4(float x, float y, float z, float w)
{
	m_data = _mm_set_ps(w, z, y, x);
}

T_MATH_INLINE Vector4::Vector4(const float* p)
{
	T_ASSERT (p);
	m_data = _mm_loadu_ps(p);
}

T_MATH_INLINE const Vector4& Vector4::zero()
{
	static const Vector4 zero(0.0f, 0.0f, 0.0f, 0.0f);
	return zero;
}

T_MATH_INLINE const Vector4& Vector4::one()
{
	static const Vector4 one(1.0f, 1.0f, 1.0f, 1.0f);
	return one;
}

T_MATH_INLINE const Vector4& Vector4::origo()
{
	static const Vector4 origo(0.0f, 0.0f, 0.0f, 1.0f);
	return origo;
}

T_MATH_INLINE void Vector4::set(float x, float y, float z, float w)
{
	m_data = _mm_set_ps(w, z, y, x);
}

T_MATH_INLINE Scalar Vector4::x() const
{
	Vector4 tmp = shuffle< 0, 0, 0, 0 >();
	return Scalar(tmp.m_data);
}

T_MATH_INLINE Scalar Vector4::y() const
{
	Vector4 tmp = shuffle< 1, 1, 1, 1 >();
	return Scalar(tmp.m_data);
}

T_MATH_INLINE Scalar Vector4::z() const
{
	Vector4 tmp = shuffle< 2, 2, 2, 2 >();
	return Scalar(tmp.m_data);
}

T_MATH_INLINE Scalar Vector4::w() const
{
	Vector4 tmp = shuffle< 3, 3, 3, 3 >();
	return Scalar(tmp.m_data);
}

T_MATH_INLINE Vector4 Vector4::xyz0() const
{
	static const Vector4 c_mask(1.0f, 1.0f, 1.0f, 0.0f);
	return *this * c_mask; 
}

T_MATH_INLINE Vector4 Vector4::xyz1() const
{
	static const Vector4 c_wone(0.0f, 0.0f, 0.0f, 1.0f);
	return xyz0() + c_wone;
}

T_MATH_INLINE Scalar Vector4::length() const
{
	Scalar ln2 = length2();
	return Scalar(_mm_sqrt_ps(ln2.m_data));
}

T_MATH_INLINE Scalar Vector4::length2() const
{
	return dot4(*this, *this);
}

T_MATH_INLINE Vector4 Vector4::normalized() const
{
	return *this / length();
}

T_MATH_INLINE Vector4 Vector4::absolute() const
{
	static const uint32_t T_ALIGN16 c_absoluteMask[] = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	__m128 mask = _mm_load_ps((const float *)c_absoluteMask);
	return Vector4(_mm_and_ps(m_data, mask));
}

T_MATH_INLINE void Vector4::storeAligned(float* out) const
{
	T_ASSERT (out);
	_mm_store_ps(out, m_data);
}

T_MATH_INLINE void Vector4::storeUnaligned(float* out) const
{
	T_ASSERT (out);
	_mm_storeu_ps(out, m_data);
}

T_MATH_INLINE Scalar Vector4::get(int index) const
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

T_MATH_INLINE void Vector4::set(int index, const Scalar& value)
{
	switch (index)
	{
	case 0:
		*this = *this * Vector4(0.0f, 1.0f, 1.0f, 1.0f) + Vector4(value, 0.0f, 0.0f, 0.0f);
		break;
	case 1:
		*this = *this * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(0.0f, value, 0.0f, 0.0f);
		break;
	case 2:
		*this = *this * Vector4(1.0f, 1.0f, 0.0f, 1.0f) + Vector4(0.0f, 0.0f, value, 0.0f);
		break;
	case 3:
		*this = *this * Vector4(1.0f, 1.0f, 1.0f, 0.0f) + Vector4(0.0f, 0.0f, 0.0f, value);
		break;
	}
}

T_MATH_INLINE Vector4& Vector4::operator = (const Vector4& v)
{
	m_data = v.m_data;
	return *this;
}

T_MATH_INLINE Vector4 Vector4::operator - () const
{
	static const uint32_t T_ALIGN16 c_negateMask[] = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
	__m128 mask = _mm_load_ps((const float *)c_negateMask);
	return Vector4(_mm_xor_ps(m_data, mask));
}

T_MATH_INLINE Vector4& Vector4::operator += (const Scalar& v)
{
	m_data = _mm_add_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator += (const Vector4& v)
{
	m_data = _mm_add_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Scalar& v)
{
	m_data = _mm_sub_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Vector4& v)
{
	m_data = _mm_sub_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Scalar& v)
{
	m_data = _mm_mul_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Vector4& v)
{
	m_data = _mm_mul_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Scalar& v)
{
	m_data = _mm_div_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Vector4& v)
{
	m_data = _mm_div_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE bool Vector4::operator == (const Vector4& v) const
{
	Vector4 diff = (*this - v).absolute();

	float T_ALIGN16 tmp[4];
	_mm_store_ps(tmp, diff.m_data);

	return
		tmp[0] <= 1e-3f &&
		tmp[1] <= 1e-3f &&
		tmp[2] <= 1e-3f &&
		tmp[3] <= 1e-3f;
}

T_MATH_INLINE bool Vector4::operator != (const Vector4& v) const
{
	Vector4 diff = (*this - v).absolute();

	float T_ALIGN16 tmp[4];
	_mm_store_ps(tmp, diff.m_data);

	return
		tmp[0] > 1e-3f ||
		tmp[1] > 1e-3f ||
		tmp[2] > 1e-3f ||
		tmp[3] > 1e-3f;
}

T_MATH_INLINE Scalar Vector4::operator [] (int index) const
{
	return get(index);
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Scalar& r)
{
	return Vector4(_mm_add_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Scalar& l, const Vector4& r)
{
	return Vector4(_mm_add_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Vector4& r)
{
	return Vector4(_mm_add_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Scalar& r)
{
	return Vector4(_mm_sub_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Scalar& l, const Vector4& r)
{
	return Vector4(_mm_sub_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Vector4& r)
{
	return Vector4(_mm_sub_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Scalar& r)
{
	return Vector4(_mm_mul_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Scalar& l, const Vector4& r)
{
	return Vector4(_mm_mul_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Vector4& r)
{
	return Vector4(_mm_mul_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Scalar& r)
{
	return Vector4(_mm_div_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator / (const Scalar& l, const Vector4& r)
{
	return Vector4(_mm_div_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Vector4& r)
{
	return Vector4(_mm_div_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar dot3(const Vector4& l, const Vector4& r)
{
	Vector4 tmp = l * r;
	return tmp.x() + tmp.y() + tmp.z();
}

T_MATH_INLINE Scalar dot4(const Vector4& l, const Vector4& r)
{
	Vector4 tmp = l * r;
	return tmp.x() + tmp.y() + tmp.z() + tmp.w();
}

T_MATH_INLINE Vector4 cross(const Vector4& l, const Vector4& r)
{
	Vector4 tmp1 = l.shuffle< 1, 2, 0, 3 >();
	Vector4 tmp2 = r.shuffle< 2, 0, 1, 3 >();
	Vector4 tmp3 = l.shuffle< 2, 0, 1, 3 >();
	Vector4 tmp4 = r.shuffle< 1, 2, 0, 3 >();
	return tmp1 * tmp2 - tmp3 * tmp4;
}

T_MATH_INLINE Vector4 lerp(const Vector4& a, const Vector4& b, const Scalar& c)
{
	return (Scalar(1.0f) - c) * a + c * b;
}

T_MATH_INLINE Vector4 reflect(const Vector4& v, const Vector4& at)
{
	const static Scalar c_two(2.0f);
	Vector4 N = at.normalized();
	Vector4 V = N * (dot3(N, v) * c_two);
	return V - v;
}

T_MATH_INLINE int majorAxis3(const Vector4& v)
{
	if (abs(v.x()) > abs(v.y()))
		return (abs(v.x()) > abs(v.z())) ? 0 : 2;
	return (abs(v.y()) > abs(v.z())) ? 1 : 2;
}

T_MATH_INLINE Vector4 min(const Vector4& l, const Vector4& r)
{
	return Vector4(_mm_min_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 max(const Vector4& l, const Vector4& r)
{
	return Vector4(_mm_max_ps(l.m_data, r.m_data));
}

}

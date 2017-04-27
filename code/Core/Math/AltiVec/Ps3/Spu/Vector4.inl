/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace
	{

vec_float4 v_vec_div(vec_float4 divend, vec_float4 denom)
{
	vec_float4 idenom = spu_re(denom);
	vec_float4 res0 = spu_madd(divend, idenom, (vec_float4)(0.0f));
	return spu_madd(
		spu_nmsub(denom, idenom, (vec_float4)(1.0f)),
		res0,
		res0
	);
}

	}

T_MATH_INLINE Vector4::Vector4()
{
}

T_MATH_INLINE Vector4::Vector4(const Vector4& v)
:	m_data(v.m_data)
{
}

T_MATH_INLINE Vector4::Vector4(const Scalar& s)
:	m_data(s.m_data)
{
}

T_MATH_INLINE Vector4::Vector4(vec_float4 v)
:	m_data(v)
{
}

T_MATH_INLINE Vector4::Vector4(float x, float y, float z, float w)
{
	m_data = (vec_float4){ x, y, z, w };
}

T_MATH_INLINE Vector4::Vector4(const float* p)
{
	T_ASSERT (p);
	m_data = (vec_float4){ p[0], p[1], p[2], p[3] };
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
	m_data = (vec_float4){ x, y, z, w };
}

T_MATH_INLINE Scalar Vector4::x() const
{
	return Scalar(spu_extract(m_data, 0));
}

T_MATH_INLINE Scalar Vector4::y() const
{
	return Scalar(spu_extract(m_data, 1));
}

T_MATH_INLINE Scalar Vector4::z() const
{
	return Scalar(spu_extract(m_data, 2));
}

T_MATH_INLINE Scalar Vector4::w() const
{
	return Scalar(spu_extract(m_data, 3));
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
	return squareRoot(ln2);
}

T_MATH_INLINE Scalar Vector4::length2() const
{
	return dot4(*this, *this);
}

T_MATH_INLINE Vector4 Vector4::normalized() const
{
	Scalar il = reciprocalSquareRoot(dot4(*this, *this));
	return *this * il;
}

T_MATH_INLINE Vector4 Vector4::absolute() const
{
	vec_uint4 umask = (vec_uint4){ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	return Vector4(spu_and(m_data, (vec_float4)umask));
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
	*(vec_float4*)out = m_data;
}

T_MATH_INLINE void Vector4::storeUnaligned(float* out) const
{
	T_ASSERT (out);
	out[0] = spu_extract(m_data, 0);
	out[1] = spu_extract(m_data, 1);
	out[2] = spu_extract(m_data, 2);
	out[3] = spu_extract(m_data, 3);
}

T_MATH_INLINE Scalar Vector4::get(int index) const
{
	return Scalar(spu_extract(m_data, index));
}

T_MATH_INLINE void Vector4::set(int index, const Scalar& value)
{
	m_data = spu_insert(value, m_data, index);
}

T_MATH_INLINE Vector4& Vector4::operator = (const Vector4& v)
{
	m_data = v.m_data;
	return *this;
}

T_MATH_INLINE Vector4 Vector4::operator - () const
{
	vec_uint4 umask = (vec_uint4){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
	return Vector4(spu_xor(m_data, (vec_float4)umask));
}

T_MATH_INLINE Vector4& Vector4::operator += (const Scalar& v)
{
	m_data = spu_add(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator += (const Vector4& v)
{
	m_data = spu_add(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Scalar& v)
{
	m_data = spu_sub(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Vector4& v)
{
	m_data = spu_sub(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Scalar& v)
{
	m_data = spu_madd(m_data, v.m_data, (vec_float4)(0.0f));
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Vector4& v)
{
	m_data = spu_madd(m_data, v.m_data, (vec_float4)(0.0f));
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Scalar& v)
{
	m_data = v_vec_div(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Vector4& v)
{
	m_data = v_vec_div(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE bool Vector4::operator == (const Vector4& v) const
{
	Vector4 diff = (*this - v).absolute();
	return
		diff.m_data[0] <= 1e-3f &&
		diff.m_data[1] <= 1e-3f &&
		diff.m_data[2] <= 1e-3f &&
		diff.m_data[3] <= 1e-3f;
}

T_MATH_INLINE bool Vector4::operator != (const Vector4& v) const
{
	Vector4 diff = (*this - v).absolute();
	return
		diff.m_data[0] > 1e-3f ||
		diff.m_data[1] > 1e-3f ||
		diff.m_data[2] > 1e-3f ||
		diff.m_data[3] > 1e-3f;
}

T_MATH_INLINE Scalar Vector4::operator [] (int index) const
{
	return get(index);
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Scalar& r)
{
	return Vector4(spu_add(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Scalar& l, const Vector4& r)
{
	return Vector4(spu_add(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Vector4& r)
{
	return Vector4(spu_add(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Scalar& r)
{
	return Vector4(spu_sub(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Scalar& l, const Vector4& r)
{
	return Vector4(spu_sub(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Vector4& r)
{
	return Vector4(spu_sub(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Scalar& r)
{
	return Vector4(spu_madd(l.m_data, r.m_data, (vec_float4)(0.0f)));
}

T_MATH_INLINE Vector4 operator * (const Scalar& l, const Vector4& r)
{
	return Vector4(spu_madd(l.m_data, r.m_data, (vec_float4)(0.0f)));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Vector4& r)
{
	return Vector4(spu_madd(l.m_data, r.m_data, (vec_float4)(0.0f)));
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Scalar& r)
{
	return Vector4(v_vec_div(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator / (const Scalar& l, const Vector4& r)
{
	return Vector4(v_vec_div(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Vector4& r)
{
	return Vector4(v_vec_div(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar horizontalAdd3(const Vector4& v)
{
	return horizontalAdd4(v.xyz0());
}

T_MATH_INLINE Scalar horizontalAdd4(const Vector4& v)
{
	Vector4 tmp = v + v.shuffle< 1, 0, 3, 2 >();
	return Scalar((tmp + tmp.shuffle< 2, 2, 0, 0 >()).m_data);
}

T_MATH_INLINE Scalar dot3(const Vector4& l, const Vector4& r)
{
	return horizontalAdd3(l * r);
}

T_MATH_INLINE Scalar dot4(const Vector4& l, const Vector4& r)
{
	return horizontalAdd4(l * r);
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
	// \fixme
	return l;//Vector4(spu_min(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 max(const Vector4& l, const Vector4& r)
{
	// \fixme
	return l;//Vector4(spu_max(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 clamp(const Vector4& value, const Vector4& minLimit, const Vector4& maxLimit)
{
	// \fixme
	return value;
}

T_MATH_INLINE Vector4 select(const Vector4& condition, const Vector4& negative, const Vector4& positive)
{
	// \fixme
	return positive;
}

T_MATH_INLINE bool compareAllGreaterEqual(const Vector4& l, const Vector4& r)
{
	// \fixme
	return false;
}

T_MATH_INLINE bool compareAllLessEqual(const Vector4& l, const Vector4& r)
{
	// \fixme
	return false;
}
}

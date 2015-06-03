#include <cmath>
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace
	{

static const Vector4 c_zero(0.0f, 0.0f, 0.0f, 0.0f);
static const Vector4 c_one(1.0f, 1.0f, 1.0f, 1.0f);
static const Vector4 c_origo(0.0f, 0.0f, 0.0f, 1.0f);

float32x4_t v_vec_div(float32x4_t divend, float32x4_t denom)
{
	/*
	const float32x4_t one = { 1.0f, 1.0f, 1.0f, 1.0f };

	float32x4_t idenom = vrecpeq_f32(denom);
	float32x4_t res0 = vmulq_f32(divend, idenom);
	
	float32x4_t t0 = vmulq_f32(denom, idenom);
	float32x4_t t1 = vsubq_f32(t0, one);
	float32x4_t t2 = vnegq_f32(t1);
	
	// r0 + r0 * t2
	return vmlaq_f32(
		res0,
		res0,
		t2
	);
	*/

	float T_MATH_ALIGN16 dv[4];
	float T_MATH_ALIGN16 de[4];

	vst1q_f32((float32_t*)dv, divend);
	vst1q_f32((float32_t*)de, denom);

	return (float32x4_t){
		dv[0] / de[0],
		dv[1] / de[1],
		dv[2] / de[2],
		dv[3] / de[3]
	};
}

inline bool compare(float e1, float e2)
{
	return abs(e2 - e1) <= 1e-8f;
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
{
	m_data = vdupq_n_f32(s.m_data);
}

T_MATH_INLINE Vector4::Vector4(float32x4_t v)
:	m_data(v)
{
}

T_MATH_INLINE Vector4::Vector4(float x, float y, float z, float w)
{
	m_data = (float32x4_t){ x, y, z, w };
}

T_MATH_INLINE Vector4::Vector4(const float* p)
{
	T_ASSERT (p);
	m_data = vld1q_f32((const float32_t*)p);
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
	m_data = (float32x4_t){ x, y, z, w };
}

T_MATH_INLINE Scalar Vector4::x() const
{
	return Scalar(vgetq_lane_f32(m_data, 0));
}

T_MATH_INLINE Scalar Vector4::y() const
{
	return Scalar(vgetq_lane_f32(m_data, 1));
}

T_MATH_INLINE Scalar Vector4::z() const
{
	return Scalar(vgetq_lane_f32(m_data, 2));
}

T_MATH_INLINE Scalar Vector4::w() const
{
	return Scalar(vgetq_lane_f32(m_data, 3));
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

T_MATH_INLINE Scalar Vector4::normalize()
{
	Scalar ln = length();
	T_ASSERT (abs(ln) > 0.0f);
	*this /= ln;
	return ln;
}

T_MATH_INLINE Vector4 Vector4::normalized() const
{
	Scalar il = reciprocalSquareRoot(dot4(*this, *this));
	T_ASSERT (abs(il) > 0.0f);
	return *this * il;
}

T_MATH_INLINE Vector4 Vector4::absolute() const
{
	return Vector4(vabsq_f32(m_data));
}

T_MATH_INLINE Vector4 Vector4::loadAligned(const float* in)
{
	T_ASSERT (in);
	Vector4 v; v.m_data = vld1q_f32((const float32_t*)in);
	return v;
}

T_MATH_INLINE Vector4 Vector4::loadUnaligned(const float* in)
{
	T_ASSERT (in);
	Vector4 v; v.m_data = vld1q_f32((const float32_t*)in);
	return v;
}

T_MATH_INLINE void Vector4::storeAligned(float* out) const
{
	T_ASSERT (out);
	vst1q_f32((float32_t*)out, m_data);
}

T_MATH_INLINE void Vector4::storeUnaligned(float* out) const
{
	T_ASSERT (out);
	vst1q_f32((float32_t*)out, m_data);
}

T_MATH_INLINE Scalar Vector4::get(int index) const
{
	return Scalar(*((const float *)&m_data + index));
}

T_MATH_INLINE void Vector4::set(int index, const Scalar& value)
{
	*((float*)&m_data + index) = value;
}

T_MATH_INLINE Vector4& Vector4::operator = (const Vector4& v)
{
	m_data = v.m_data;
	return *this;
}

T_MATH_INLINE Vector4 Vector4::operator - () const
{
	return Vector4(vnegq_f32(m_data));
}

T_MATH_INLINE Vector4& Vector4::operator += (const Scalar& v)
{
	float32x4_t v4 = vdupq_n_f32(v.m_data);
	m_data = vaddq_f32(m_data, v4);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator += (const Vector4& v)
{
	m_data = vaddq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Scalar& v)
{
	float32x4_t v4 = vdupq_n_f32(v.m_data);
	m_data = vsubq_f32(m_data, v4);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Vector4& v)
{
	m_data = vsubq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Scalar& v)
{
	float32x4_t v4 = vdupq_n_f32(v.m_data);
	m_data = vmulq_f32(m_data, v4);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Vector4& v)
{
	m_data = vmulq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Scalar& v)
{
	T_ASSERT (abs(v.m_data) > 0.0f);
	float32x4_t v4 = vdupq_n_f32(v.m_data);
	m_data = v_vec_div(m_data, v4);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator /= (const Vector4& v)
{
	m_data = v_vec_div(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE bool Vector4::operator == (const Vector4& v) const
{
	float T_MATH_ALIGN16 le[4];
	float T_MATH_ALIGN16 re[4];

	storeAligned(le);
	v.storeAligned(re);

	return bool(
		compare(le[0], re[0]) &&
		compare(le[1], re[1]) &&
		compare(le[2], re[2]) &&
		compare(le[3], re[3])
	);	
}

T_MATH_INLINE bool Vector4::operator != (const Vector4& v) const
{
	float T_MATH_ALIGN16 le[4];
	float T_MATH_ALIGN16 re[4];

	storeAligned(le);
	v.storeAligned(re);

	return bool(
		!compare(le[0], re[0]) ||
		!compare(le[1], re[1]) ||
		!compare(le[2], re[2]) ||
		!compare(le[3], re[3])
	);	
}

T_MATH_INLINE Scalar Vector4::operator [] (int index) const
{
	return get(index);
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Scalar& r)
{
	float32x4_t v4 = vdupq_n_f32(r.m_data);
	return Vector4(vaddq_f32(l.m_data, v4));
}

T_MATH_INLINE Vector4 operator + (const Scalar& l, const Vector4& r)
{
	float32x4_t v4 = vdupq_n_f32(l.m_data);
	return Vector4(vaddq_f32(v4, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Vector4& r)
{
	return Vector4(vaddq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Scalar& r)
{
	float32x4_t v4 = vdupq_n_f32(r.m_data);
	return Vector4(vsubq_f32(l.m_data, v4));
}

T_MATH_INLINE Vector4 operator - (const Scalar& l, const Vector4& r)
{
	float32x4_t v4 = vdupq_n_f32(l.m_data);
	return Vector4(vsubq_f32(v4, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Vector4& r)
{
	return Vector4(vsubq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Scalar& r)
{
	float32x4_t v4 = vdupq_n_f32(r.m_data);
	return Vector4(vmulq_f32(l.m_data, v4));
}

T_MATH_INLINE Vector4 operator * (const Scalar& l, const Vector4& r)
{
	float32x4_t v4 = vdupq_n_f32(l.m_data);
	return Vector4(vmulq_f32(v4, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Vector4& r)
{
	return Vector4(vmulq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator / (const Vector4& l, const Scalar& r)
{
	T_ASSERT (abs(r.m_data) > 0.0);
	float32x4_t v4 = vdupq_n_f32(r.m_data);
	return Vector4(v_vec_div(l.m_data, v4));
}

T_MATH_INLINE Vector4 operator / (const Scalar& l, const Vector4& r)
{
	float32x4_t v4 = vdupq_n_f32(l.m_data);
	return Vector4(v_vec_div(v4, r.m_data));
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
	return (tmp + tmp.shuffle< 2, 2, 0, 0 >()).x();
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
	float T_MATH_ALIGN16 e[4];
	v.absolute().storeAligned(e);
	if (e[0] > e[1])
	{
		if (e[0] > e[2])
			return 0;
		else
			return 2;
	}
	else
	{
		if (e[1] > e[2])
			return 1;
		else
			return 2;
	}
}

T_MATH_INLINE void orthogonalFrame(const Vector4& d, Vector4& outU, Vector4& outV)
{
	const static Vector4 c_axises[] =
	{
		Vector4(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, 1.0f, 0.0f),
		Vector4(1.0f, 0.0f, 0.0f, 0.0f)
	};
	int m = majorAxis3(d);
	outU = cross(d, c_axises[m]).normalized();
	outV = cross(outU, d).normalized();
}

T_MATH_INLINE Vector4 min(const Vector4& l, const Vector4& r)
{
	return Vector4(vminq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 max(const Vector4& l, const Vector4& r)
{
	return Vector4(vmaxq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 clamp(const Vector4& value, const Vector4& minLimit, const Vector4& maxLimit)
{
	return min(max(value, minLimit), maxLimit);
}

T_MATH_INLINE Vector4 select(const Vector4& condition, const Vector4& negative, const Vector4& positive)
{
	const float32x4_t zero = { 0.0f, 0.0f, 0.0f, 0.0f };
	uint32x4_t c = vcgeq_f32(condition.m_data, zero);
	float32x4_t r = vbslq_f32(c, positive.m_data, negative.m_data);
	return Vector4(r);
}

T_MATH_INLINE T_DLLCLASS bool compareAllGreaterEqual(const Vector4& l, const Vector4& r)
{
	const float* p1 = (const float*)&l.m_data;
	const float* p2 = (const float*)&r.m_data;
	return p1[0] >= p2[0] && p1[1] >= p2[1] && p1[2] >= p2[2] && p1[3] >= p2[3];
}

T_MATH_INLINE T_DLLCLASS bool compareAllLessEqual(const Vector4& l, const Vector4& r)
{
	const float* p1 = (const float*)&l.m_data;
	const float* p2 = (const float*)&r.m_data;
	return p1[0] <= p2[0] && p1[1] <= p2[1] && p1[2] <= p2[2] && p1[3] <= p2[3];
}

}

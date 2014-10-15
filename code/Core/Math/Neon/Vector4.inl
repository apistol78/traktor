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
	m_data = (float32x4_t){ p[0], p[1], p[2], p[3] };
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

T_MATH_INLINE Vector4 Vector4::normalized() const
{
	Scalar il = reciprocalSquareRoot(dot4(*this, *this));
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
	float e[4];
	storeUnaligned(e);
	return Scalar(e[index]);
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
	m_data = vaddq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator += (const Vector4& v)
{
	m_data = vaddq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Scalar& v)
{
	m_data = vsubq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator -= (const Vector4& v)
{
	m_data = vsubq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Scalar& v)
{
	m_data = vmulq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Vector4& Vector4::operator *= (const Vector4& v)
{
	m_data = vmulq_f32(m_data, v.m_data);
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
	const float* p1 = (const float*)&m_data;
	const float* p2 = (const float*)&v.m_data;
	return p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2] && p1[3] == p2[3];
}

T_MATH_INLINE bool Vector4::operator != (const Vector4& v) const
{
	return !(*this == v);
}

T_MATH_INLINE Scalar Vector4::operator [] (int index) const
{
	return get(index);
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Scalar& r)
{
	return Vector4(vaddq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Scalar& l, const Vector4& r)
{
	return Vector4(vaddq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator + (const Vector4& l, const Vector4& r)
{
	return Vector4(vaddq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Scalar& r)
{
	return Vector4(vsubq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Scalar& l, const Vector4& r)
{
	return Vector4(vsubq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator - (const Vector4& l, const Vector4& r)
{
	return Vector4(vsubq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Scalar& r)
{
	return Vector4(vmulq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Scalar& l, const Vector4& r)
{
	return Vector4(vmulq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 operator * (const Vector4& l, const Vector4& r)
{
	return Vector4(vmulq_f32(l.m_data, r.m_data));
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
	else
		return (abs(v.y()) > abs(v.z())) ? 1 : 2;
}

T_MATH_INLINE Vector4 min(const Vector4& l, const Vector4& r)
{
	return Vector4(vminq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Vector4 max(const Vector4& l, const Vector4& r)
{
	return Vector4(vmaxq_f32(l.m_data, r.m_data));
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

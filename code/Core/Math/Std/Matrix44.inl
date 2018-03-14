/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Math/Matrix44.h"
#include "Core/Math/Const.h"

namespace traktor
{

T_MATH_INLINE Matrix44::Matrix44()
{
}

T_MATH_INLINE Matrix44::Matrix44(const Matrix44& m)
{
	for (uint32_t i = 0; i < sizeof_array(m_c); ++i)
		this->m_c[i] = m.m_c[i];
}

T_MATH_INLINE Matrix44::Matrix44(const Vector4& axisX, const Vector4& axisY, const Vector4& axisZ, const Vector4& translation)
{
	m_c[0] = axisX;
	m_c[1] = axisY;
	m_c[2] = axisZ;
	m_c[3] = translation;
}

T_MATH_INLINE Matrix44::Matrix44(
	float e11, float e12, float e13, float e14,
	float e21, float e22, float e23, float e24,
	float e31, float e32, float e33, float e34,
	float e41, float e42, float e43, float e44
)
{
	m_c[0] = Vector4(e11, e21, e31, e41);
	m_c[1] = Vector4(e12, e22, e32, e42);
	m_c[2] = Vector4(e13, e23, e33, e43);
	m_c[3] = Vector4(e14, e24, e34, e44);
}

T_MATH_INLINE const Matrix44& Matrix44::zero()
{
	static const Matrix44 zero(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);
	return zero;
}

T_MATH_INLINE const Matrix44& Matrix44::identity()
{
	static const Matrix44 identity(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	return identity;
}

T_MATH_INLINE Vector4 Matrix44::axisX() const
{
	return m_c[0];
}

T_MATH_INLINE Vector4 Matrix44::axisY() const
{
	return m_c[1];
}

T_MATH_INLINE Vector4 Matrix44::axisZ() const
{
	return m_c[2];
}

T_MATH_INLINE Vector4 Matrix44::translation() const
{
	return m_c[3];
}

T_MATH_INLINE Vector4 Matrix44::diagonal() const
{
	return Vector4(m_c[0].x(), m_c[1].y(), m_c[2].z(), m_c[3].w());
}

T_MATH_INLINE bool Matrix44::isOrtho() const
{
	return abs(determinant()) == 1.0f;
}

T_MATH_INLINE Scalar Matrix44::determinant() const
{
	Vector4 c0_xxyz = m_c[0].shuffle< 0, 0, 1, 2 >();
	Vector4 c0_ywzw = m_c[0].shuffle< 1, 3, 2, 3 >();
	Vector4 c1_xxyz = m_c[1].shuffle< 0, 0, 1, 2 >();
	Vector4 c1_ywzw = m_c[1].shuffle< 1, 3, 2, 3 >();
	Vector4 c2_zyxx = m_c[2].shuffle< 2, 1, 0, 0 >();
	Vector4 c2_wzwy = m_c[2].shuffle< 3, 2, 3, 1 >();
	Vector4 c3_zyxx = m_c[3].shuffle< 2, 1, 0, 0 >();
	Vector4 c3_wzwy = m_c[3].shuffle< 3, 2, 3, 1 >();

	Vector4 c0_xy = m_c[0].shuffle< 0, 1, 0, 0 >();
	Vector4 c0_zw = m_c[0].shuffle< 2, 3, 0, 0 >();
	Vector4 c1_xy = m_c[1].shuffle< 0, 1, 0, 0 >();
	Vector4 c1_zw = m_c[1].shuffle< 2, 3, 0, 0 >();
	Vector4 c2_yx = m_c[2].shuffle< 1, 0, 0, 0 >();
	Vector4 c3_yx = m_c[3].shuffle< 1, 0, 0, 0 >();

	Vector4 pos = (c0_xxyz * c1_ywzw - c0_ywzw * c1_xxyz) * (c2_zyxx * c3_wzwy - c2_wzwy * c3_zyxx);
	Vector4 neg = (c0_xy * c1_zw - c0_zw * c1_xy) * (c2_yx * c3_wzwy - c2_wzwy * c3_yx);

	return pos.x() + pos.y() + pos.z() + pos.w() - neg.x() - neg.y();
}

T_MATH_INLINE Matrix44 Matrix44::transpose() const
{
#if defined(T_MATH_USE_SSE2)

	__m128 t0 = _mm_unpacklo_ps(m_c[0].m_data, m_c[1].m_data);	// c0x,c1x,c0y,c1y
	__m128 t1 = _mm_unpackhi_ps(m_c[0].m_data, m_c[1].m_data);	// c0z,c1z,c0w,c1w
	__m128 t2 = _mm_unpacklo_ps(m_c[2].m_data, m_c[3].m_data);	// c2x,c3x,c2y,c3y
	__m128 t3 = _mm_unpackhi_ps(m_c[2].m_data, m_c[3].m_data);	// c2z,c3z,c2w,c3w
	
	Matrix44 Mt;
	Mt.m_c[0].m_data = _mm_movelh_ps(t0, t2);	// c0x,c1x,c2x,c3x
	Mt.m_c[1].m_data = _mm_movehl_ps(t2, t0);	// c0y,c1y,c2y,c3y
	Mt.m_c[2].m_data = _mm_movelh_ps(t1, t3);	// c0z,c1z,c2z,c3z
	Mt.m_c[3].m_data = _mm_movehl_ps(t3, t1);	// c0w,c1w,c2w,c3w

	return Mt;

#elif defined(T_MATH_USE_ALTIVEC)

	vec_float4 tmp0 = vec_mergeh(m_c[0].m_data, m_c[2].m_data);
	vec_float4 tmp1 = vec_mergeh(m_c[1].m_data, m_c[3].m_data);
	vec_float4 tmp2 = vec_mergel(m_c[0].m_data, m_c[2].m_data);
	vec_float4 tmp3 = vec_mergel(m_c[1].m_data, m_c[3].m_data);

	Matrix44 Mt;
	Mt.m_c[0].m_data = vec_mergeh(tmp0, tmp1);
	Mt.m_c[1].m_data = vec_mergel(tmp0, tmp1);
	Mt.m_c[2].m_data = vec_mergeh(tmp2, tmp3);
	Mt.m_c[3].m_data = vec_mergel(tmp2, tmp3);

	return Mt;

#else

	return Matrix44(
		m_c[0].x(), m_c[0].y(), m_c[0].z(), m_c[0].w(),
		m_c[1].x(), m_c[1].y(), m_c[1].z(), m_c[1].w(),
		m_c[2].x(), m_c[2].y(), m_c[2].z(), m_c[2].w(),
		m_c[3].x(), m_c[3].y(), m_c[3].z(), m_c[3].w()
	);

#endif
}

T_MATH_INLINE Matrix44 Matrix44::inverse() const
{
	Matrix44 Mt = transpose();

	Scalar s = Mt.determinant();
	if (s == 0.0f)
		return identity();

	s = Scalar(1.0f) / s;

	Vector4 c0_zxxz = Mt.m_c[0].shuffle< 2, 0, 0, 2 >();
	Vector4 c0_wwyy = Mt.m_c[0].shuffle< 3, 3, 1, 1 >();
	Vector4 c0_yxwz = Mt.m_c[0].shuffle< 1, 0, 3, 2 >();
	Vector4 c0_wzyx = Mt.m_c[0].shuffle< 3, 2, 1, 0 >();
	Vector4 c0_zzxx = Mt.m_c[0].shuffle< 2, 2, 0, 0 >();
	Vector4 c0_ywwy = Mt.m_c[0].shuffle< 1, 3, 3, 1 >();
	Vector4 c0_yzwx = Mt.m_c[0].shuffle< 1, 2, 3, 0 >();
	Vector4 c0_zwxy = Mt.m_c[0].shuffle< 2, 3, 0, 1 >();
	Vector4 c0_wxyz = Mt.m_c[0].shuffle< 3, 0, 1, 2 >();

	Vector4 c1_zxxz = Mt.m_c[1].shuffle< 2, 0, 0, 2 >();
	Vector4 c1_wwyy = Mt.m_c[1].shuffle< 3, 3, 1, 1 >();
	Vector4 c1_yxwz = Mt.m_c[1].shuffle< 1, 0, 3, 2 >();
	Vector4 c1_wzyx = Mt.m_c[1].shuffle< 3, 2, 1, 0 >();
	Vector4 c1_zzxx = Mt.m_c[1].shuffle< 2, 2, 0, 0 >();
	Vector4 c1_ywwy = Mt.m_c[1].shuffle< 1, 3, 3, 1 >();
	Vector4 c1_yzwx = Mt.m_c[1].shuffle< 1, 2, 3, 0 >();
	Vector4 c1_zwxy = Mt.m_c[1].shuffle< 2, 3, 0, 1 >();
	Vector4 c1_wxyz = Mt.m_c[1].shuffle< 3, 0, 1, 2 >();

	Vector4 c2_zxxz = Mt.m_c[2].shuffle< 2, 0, 0, 2 >();
	Vector4 c2_wwyy = Mt.m_c[2].shuffle< 3, 3, 1, 1 >();
	Vector4 c2_yxwz = Mt.m_c[2].shuffle< 1, 0, 3, 2 >();
	Vector4 c2_wzyx = Mt.m_c[2].shuffle< 3, 2, 1, 0 >();
	Vector4 c2_zzxx = Mt.m_c[2].shuffle< 2, 2, 0, 0 >();
	Vector4 c2_ywwy = Mt.m_c[2].shuffle< 1, 3, 3, 1 >();
	Vector4 c2_yzwx = Mt.m_c[2].shuffle< 1, 2, 3, 0 >();
	Vector4 c2_zwxy = Mt.m_c[2].shuffle< 2, 3, 0, 1 >();
	Vector4 c2_wxyz = Mt.m_c[2].shuffle< 3, 0, 1, 2 >();

	Vector4 c3_zxxz = Mt.m_c[3].shuffle< 2, 0, 0, 2 >();
	Vector4 c3_wwyy = Mt.m_c[3].shuffle< 3, 3, 1, 1 >();
	Vector4 c3_yxwz = Mt.m_c[3].shuffle< 1, 0, 3, 2 >();
	Vector4 c3_wzyx = Mt.m_c[3].shuffle< 3, 2, 1, 0 >();
	Vector4 c3_zzxx = Mt.m_c[3].shuffle< 2, 2, 0, 0 >();
	Vector4 c3_ywwy = Mt.m_c[3].shuffle< 1, 3, 3, 1 >();
	Vector4 c3_yzwx = Mt.m_c[3].shuffle< 1, 2, 3, 0 >();
	Vector4 c3_zwxy = Mt.m_c[3].shuffle< 2, 3, 0, 1 >();
	Vector4 c3_wxyz = Mt.m_c[3].shuffle< 3, 0, 1, 2 >();

	Vector4 xxxx = c1_yzwx * (c2_zxxz * c3_wwyy - c2_wwyy * c3_zxxz) + c1_zwxy * (c2_wzyx * c3_yxwz - c2_yxwz * c3_wzyx) + c1_wxyz * (c2_ywwy * c3_zzxx - c2_zzxx * c3_ywwy);
	Vector4 yyyy = c2_yzwx * (c0_zxxz * c3_wwyy - c0_wwyy * c3_zxxz) + c2_zwxy * (c0_wzyx * c3_yxwz - c0_yxwz * c3_wzyx) + c2_wxyz * (c0_ywwy * c3_zzxx - c0_zzxx * c3_ywwy);
	Vector4 zzzz = c3_yzwx * (c0_zxxz * c1_wwyy - c0_wwyy * c1_zxxz) + c3_zwxy * (c0_wzyx * c1_yxwz - c0_yxwz * c1_wzyx) + c3_wxyz * (c0_ywwy * c1_zzxx - c0_zzxx * c1_ywwy);
	Vector4 wwww = c0_yzwx * (c1_wwyy * c2_zxxz - c1_zxxz * c2_wwyy) + c0_zwxy * (c1_yxwz * c2_wzyx - c1_wzyx * c2_yxwz) + c0_wxyz * (c1_zzxx * c2_ywwy - c1_ywwy * c2_zzxx);

	xxxx *= s;
	yyyy *= s;
	zzzz *= s;
	wwww *= s;

	return Matrix44(
		Vector4(xxxx.x(), xxxx.y(), xxxx.z(), xxxx.w()),
		Vector4(yyyy.x(), yyyy.y(), yyyy.z(), yyyy.w()),
		Vector4(zzzz.x(), zzzz.y(), zzzz.z(), zzzz.w()),
		Vector4(wwww.x(), wwww.y(), wwww.z(), wwww.w())
	);
}

T_MATH_INLINE Matrix44 Matrix44::loadAligned(const float* in)
{
	Matrix44 m;
	m.m_c[0] = Vector4::loadAligned(&in[0]);
	m.m_c[1] = Vector4::loadAligned(&in[4]);
	m.m_c[2] = Vector4::loadAligned(&in[8]);
	m.m_c[3] = Vector4::loadAligned(&in[12]);
	return m;
}

T_MATH_INLINE Matrix44 Matrix44::loadUnaligned(const float* in)
{
	Matrix44 m;
	m.m_c[0] = Vector4::loadUnaligned(&in[0]);
	m.m_c[1] = Vector4::loadUnaligned(&in[4]);
	m.m_c[2] = Vector4::loadUnaligned(&in[8]);
	m.m_c[3] = Vector4::loadUnaligned(&in[12]);
	return m;
}

T_MATH_INLINE void Matrix44::storeAligned(float* out) const
{
	m_c[0].storeAligned(&out[0]);
	m_c[1].storeAligned(&out[4]);
	m_c[2].storeAligned(&out[8]);
	m_c[3].storeAligned(&out[12]);
}

T_MATH_INLINE void Matrix44::storeUnaligned(float* out) const
{
	m_c[0].storeUnaligned(&out[0]);
	m_c[1].storeUnaligned(&out[4]);
	m_c[2].storeUnaligned(&out[8]);
	m_c[3].storeUnaligned(&out[12]);
}

T_MATH_INLINE const Vector4& Matrix44::get(int c) const
{
	return m_c[c];
}

T_MATH_INLINE void Matrix44::set(int c, const Vector4& v)
{
	m_c[c] = v;
}

T_MATH_INLINE Scalar Matrix44::get(int r, int c) const
{
	return m_c[c].get(r);
}

T_MATH_INLINE void Matrix44::set(int r, int c, const Scalar& v)
{
	m_c[c].set(r, v);
}

T_MATH_INLINE Scalar Matrix44::operator () (int r, int c) const
{
	return m_c[c][r];
}

T_MATH_INLINE Matrix44& Matrix44::operator = (const Matrix44& m)
{
	for (uint32_t i = 0; i < sizeof_array(m_c); ++i)
		this->m_c[i] = m.m_c[i];
	return *this;
}

T_MATH_INLINE Matrix44& Matrix44::operator += (const Matrix44& m)
{
	*this = *this + m;
	return *this;
}

T_MATH_INLINE Matrix44& Matrix44::operator -= (const Matrix44& m)
{
	*this = *this - m;
	return *this;
}

T_MATH_INLINE Matrix44& Matrix44::operator *= (const Matrix44& m)
{
	*this = *this * m;
	return *this;
}

T_MATH_INLINE bool Matrix44::operator == (const Matrix44& m) const
{
	for (int c = 0; c < 4; ++c)
	{
		if (m_c[c] != m.m_c[c])
			return false;
	}
	return true;
}

T_MATH_INLINE bool Matrix44::operator != (const Matrix44& m) const
{
	for (int c = 0; c < 4; ++c)
	{
		if (m_c[c] != m.m_c[c])
			return true;
	}
	return false;
}

T_MATH_INLINE Matrix44 operator + (const Matrix44& lh, const Matrix44& rh)
{
	Matrix44 m;
	for (int c = 0; c < 4; ++c)
		m.m_c[c] = lh.m_c[c] + rh.m_c[c];
	return m;
}

T_MATH_INLINE Matrix44 operator - (const Matrix44& lh, const Matrix44& rh)
{
	Matrix44 m;
	for (int c = 0; c < 4; ++c)
		m.m_c[c] = lh.m_c[c] - rh.m_c[c];
	return m;
}

T_MATH_INLINE Vector4 operator * (const Matrix44& m, const Vector4& v)
{
	return v.x() * m.m_c[0] + v.y() * m.m_c[1] + v.z() * m.m_c[2] + v.w() * m.m_c[3];
}

T_MATH_INLINE Vector4 operator * (const Vector4& v, const Matrix44& m)
{
	return m.transpose() * v;
}

T_MATH_INLINE Matrix44 operator * (const Matrix44& lh, const Matrix44& rh)
{
	Vector4 c0_xxxx = rh.m_c[0].shuffle< 0, 0, 0, 0 >();
	Vector4 c0_yyyy = rh.m_c[0].shuffle< 1, 1, 1, 1 >();
	Vector4 c0_zzzz = rh.m_c[0].shuffle< 2, 2, 2, 2 >();
	Vector4 c0_wwww = rh.m_c[0].shuffle< 3, 3, 3, 3 >();

	Vector4 c1_xxxx = rh.m_c[1].shuffle< 0, 0, 0, 0 >();
	Vector4 c1_yyyy = rh.m_c[1].shuffle< 1, 1, 1, 1 >();
	Vector4 c1_zzzz = rh.m_c[1].shuffle< 2, 2, 2, 2 >();
	Vector4 c1_wwww = rh.m_c[1].shuffle< 3, 3, 3, 3 >();

	Vector4 c2_xxxx = rh.m_c[2].shuffle< 0, 0, 0, 0 >();
	Vector4 c2_yyyy = rh.m_c[2].shuffle< 1, 1, 1, 1 >();
	Vector4 c2_zzzz = rh.m_c[2].shuffle< 2, 2, 2, 2 >();
	Vector4 c2_wwww = rh.m_c[2].shuffle< 3, 3, 3, 3 >();

	Vector4 c3_xxxx = rh.m_c[3].shuffle< 0, 0, 0, 0 >();
	Vector4 c3_yyyy = rh.m_c[3].shuffle< 1, 1, 1, 1 >();
	Vector4 c3_zzzz = rh.m_c[3].shuffle< 2, 2, 2, 2 >();
	Vector4 c3_wwww = rh.m_c[3].shuffle< 3, 3, 3, 3 >();

	return Matrix44(
		c0_xxxx * lh.m_c[0] + c0_yyyy * lh.m_c[1] + c0_zzzz * lh.m_c[2] + c0_wwww * lh.m_c[3],
		c1_xxxx * lh.m_c[0] + c1_yyyy * lh.m_c[1] + c1_zzzz * lh.m_c[2] + c1_wwww * lh.m_c[3],
		c2_xxxx * lh.m_c[0] + c2_yyyy * lh.m_c[1] + c2_zzzz * lh.m_c[2] + c2_wwww * lh.m_c[3],
		c3_xxxx * lh.m_c[0] + c3_yyyy * lh.m_c[1] + c3_zzzz * lh.m_c[2] + c3_wwww * lh.m_c[3]
	);
}

T_MATH_INLINE Matrix44 orthoLh(float width, float height, float zn, float zf)
{
	return Matrix44(
		2.0f / width, 0.0f,          0.0f,             0.0f,
		0.0f,         2.0f / height, 0.0f,             0.0f,
		0.0f,         0.0f,          1.0f / (zf - zn), -zn / (zf - zn),
		0.0f,         0.0f,          0.0f,             1.0f
	);
}

T_MATH_INLINE Matrix44 orthoLh(float left, float top, float right, float bottom, float zn, float zf)
{
	return Matrix44(
		2.0f / (right - left), 0.0f,                  0.0f,             (left + right) / (left - right),
		0.0f,                  2.0f / (bottom - top), 0.0f,             (bottom + top) / (bottom - top),
		0.0f,                  0.0f,                  1.0f / (zf - zn), -zn / (zf - zn),
		0.0f,                  0.0f,                  0.0f,             1.0f
	);
}

T_MATH_INLINE Matrix44 orthoRh(float width, float height, float zn, float zf)
{
	return Matrix44(
		2.0f / width, 0.0f,          0.0f,             0.0f,
		0.0f,         2.0f / height, 0.0f,             0.0f,
		0.0f,         0.0f,          1.0f / (zn - zf), zn / (zf - zn),
		0.0f,         0.0f,          0.0f,             1.0f
	);
}

T_MATH_INLINE Matrix44 perspectiveLh(float fov, float aspect, float zn, float zf)
{
	float h = cosf(fov / 2.0f) / sinf(fov / 2.0f);
	float w = h / aspect;

	return Matrix44(
		w,    0.0f, 0.0f,			0.0f,
		0.0f, h,    0.0f,           0.0f,
		0.0f, 0.0f, zf / (zf - zn), -zn * zf / (zf - zn),
		0.0f, 0.0f, 1.0f,	        0.0f
	);
}

T_MATH_INLINE Matrix44 perspectiveRh(float fov, float aspect, float zn, float zf)
{
	float h = cosf(fov / 2.0f) / sinf(fov / 2.0f);
	float w = h / aspect;

	return Matrix44(
		w,    0.0f, 0.0f,           0.0f,
		0.0f, h,    0.0f,           0.0f,
		0.0f, 0.0f, zf / (zn - zf), -zn * zf / (zn - zf),
		0.0f, 0.0f, -1.0f,          0.0f
	);
}

T_MATH_INLINE Matrix44 translate(const Vector4& t)
{
	return Matrix44(
		1.0f, 0.0f, 0.0f, t.x(),
		0.0f, 1.0f, 0.0f, t.y(),
		0.0f, 0.0f, 1.0f, t.z(),
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 translate(float x, float y, float z)
{
	return Matrix44(
		1.0f, 0.0f, 0.0f, x,
		0.0f, 1.0f, 0.0f, y,
		0.0f, 0.0f, 1.0f, z,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 rotateX(float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);
	return Matrix44(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f,    c,   -s, 0.0f,
		0.0f,    s,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 rotateY(float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);
	return Matrix44(
		   c, 0.0f,    s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		  -s, 0.0f,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 rotateZ(float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);
	return Matrix44(
		   c,   -s, 0.0f, 0.0f,
		   s,    c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 rotate(const Vector4& axis, float angle)
{
	float cf = cosf(angle);

	Scalar c(cf);
	Scalar s(sinf(angle));
	Scalar t(1.0f - cf);

	Vector4 txyz = t * axis;

	return Matrix44(
		txyz.x() * axis.x() + c,
		txyz.x() * axis.y() + s * axis.z(),
		txyz.x() * axis.z() - s * axis.y(),
		0.0f,
		txyz.x() * axis.y() - s * axis.z(),
		txyz.y() * axis.y() + c,
		txyz.y() * axis.z() + s * axis.x(),
		0.0f,
		txyz.x() * axis.z() + s * axis.y(),
		txyz.y() * axis.z() - s * axis.x(),
		txyz.z() * axis.z() + c,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);
}

T_MATH_INLINE Matrix44 scale(const Vector4& s)
{
	return Matrix44(
		s.x(),  0.0f,  0.0f, 0.0f,
		 0.0f, s.y(),  0.0f, 0.0f,
		 0.0f,  0.0f, s.z(), 0.0f,
		 0.0f,  0.0f,  0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 scale(float x, float y, float z)
{
	return Matrix44(
		   x, 0.0f, 0.0f, 0.0f,
		0.0f,    y, 0.0f, 0.0f,
		0.0f, 0.0f,    z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 lookAt(const Vector4& position, const Vector4& target, const Vector4& up)
{
	Vector4 z = (target - position).normalized();
	Vector4 x = cross(up, z).normalized();
	Vector4 y = cross(z, x).normalized();
	return Matrix44(
		x.x(), x.y(), x.z(), -dot3(x, position),
		y.x(), y.y(), y.z(), -dot3(y, position),
		z.x(), z.y(), z.z(), -dot3(z, position),
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

}

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
	for (int i = 0; i < sizeof_array(m_r); ++i)
		this->m_r[i] = m.m_r[i];
}

T_MATH_INLINE Matrix44::Matrix44(const Vector4& axisX, const Vector4& axisY, const Vector4& axisZ, const Vector4& translation)
{
	m_r[0] = axisX;
	m_r[1] = axisY;
	m_r[2] = axisZ;
	m_r[3] = translation;
}

T_MATH_INLINE Matrix44::Matrix44(
	float e11, float e12, float e13, float e14,
	float e21, float e22, float e23, float e24,
	float e31, float e32, float e33, float e34,
	float e41, float e42, float e43, float e44
)
{
	m_r[0] = Vector4(e11, e12, e13, e14);
	m_r[1] = Vector4(e21, e22, e23, e24);
	m_r[2] = Vector4(e31, e32, e33, e34);
	m_r[3] = Vector4(e41, e42, e43, e44);
}

T_MATH_INLINE Matrix44::Matrix44(const float* p)
{
	m_r[0] = Vector4(&p[0]);
	m_r[1] = Vector4(&p[4]);
	m_r[2] = Vector4(&p[8]);
	m_r[3] = Vector4(&p[12]);
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
	return m_r[0];
}

T_MATH_INLINE Vector4 Matrix44::axisY() const
{
	return m_r[1];
}

T_MATH_INLINE Vector4 Matrix44::axisZ() const
{
	return m_r[2];
}

T_MATH_INLINE Vector4 Matrix44::translation() const
{
	return m_r[3];
}

T_MATH_INLINE Vector4 Matrix44::diagonal() const
{
	return Vector4(m_r[0].x(), m_r[1].y(), m_r[2].z(), m_r[3].w());
}

T_MATH_INLINE bool Matrix44::isOrtho() const
{
	return abs(determinant()) == 1.0f;
}

T_MATH_INLINE Scalar Matrix44::determinant() const
{
	Vector4 r0_xxyz = m_r[0].shuffle< 0, 0, 1, 2 >();
	Vector4 r0_ywzw = m_r[0].shuffle< 1, 3, 2, 3 >();
	Vector4 r1_xxyz = m_r[1].shuffle< 0, 0, 1, 2 >();
	Vector4 r1_ywzw = m_r[1].shuffle< 1, 3, 2, 3 >();
	Vector4 r2_zyxx = m_r[2].shuffle< 2, 1, 0, 0 >();
	Vector4 r2_wzwy = m_r[2].shuffle< 3, 2, 3, 1 >();
	Vector4 r3_zyxx = m_r[3].shuffle< 2, 1, 0, 0 >();
	Vector4 r3_wzwy = m_r[3].shuffle< 3, 2, 3, 1 >();

	Vector4 r0_xy = m_r[0].shuffle< 0, 1, 0, 0 >();
	Vector4 r0_zw = m_r[0].shuffle< 2, 3, 0, 0 >();
	Vector4 r1_xy = m_r[1].shuffle< 0, 1, 0, 0 >();
	Vector4 r1_zw = m_r[1].shuffle< 2, 3, 0, 0 >();
	Vector4 r2_yx = m_r[2].shuffle< 1, 0, 0, 0 >();
	Vector4 r3_yx = m_r[3].shuffle< 1, 0, 0, 0 >();

	Vector4 pos = (r0_xxyz * r1_ywzw - r0_ywzw * r1_xxyz) * (r2_zyxx * r3_wzwy - r2_wzwy * r3_zyxx);
	Vector4 neg = (r0_xy * r1_zw - r0_zw * r1_xy) * (r2_yx * r3_wzwy - r2_wzwy * r3_yx);

	return pos.x() + pos.y() + pos.z() + pos.w() - neg.x() - neg.y();
}

T_MATH_INLINE Matrix44 Matrix44::transpose() const
{
	return Matrix44(
		m_r[0].x(), m_r[1].x(), m_r[2].x(), m_r[3].x(),
		m_r[0].y(), m_r[1].y(), m_r[2].y(), m_r[3].y(),
		m_r[0].z(), m_r[1].z(), m_r[2].z(), m_r[3].z(),
		m_r[0].w(), m_r[1].w(), m_r[2].w(), m_r[3].w()
	);
}

T_MATH_INLINE Matrix44 Matrix44::inverse() const
{
	Matrix44 r;

	Scalar s = determinant();
	if (s == 0.0f)
		return identity();

	s = Scalar(1.0f) / s;

	Vector4 r0_zxxz = m_r[0].shuffle< 2, 0, 0, 2 >();
	Vector4 r0_wwyy = m_r[0].shuffle< 3, 3, 1, 1 >();
	Vector4 r0_yxwz = m_r[0].shuffle< 1, 0, 3, 2 >();
	Vector4 r0_wzyx = m_r[0].shuffle< 3, 2, 1, 0 >();
	Vector4 r0_zzxx = m_r[0].shuffle< 2, 2, 0, 0 >();
	Vector4 r0_ywwy = m_r[0].shuffle< 1, 3, 3, 1 >();
	Vector4 r0_yzwx = m_r[0].shuffle< 1, 2, 3, 0 >();
	Vector4 r0_zwxy = m_r[0].shuffle< 2, 3, 0, 1 >();
	Vector4 r0_wxyz = m_r[0].shuffle< 3, 0, 1, 2 >();

	Vector4 r1_zxxz = m_r[1].shuffle< 2, 0, 0, 2 >();
	Vector4 r1_wwyy = m_r[1].shuffle< 3, 3, 1, 1 >();
	Vector4 r1_yxwz = m_r[1].shuffle< 1, 0, 3, 2 >();
	Vector4 r1_wzyx = m_r[1].shuffle< 3, 2, 1, 0 >();
	Vector4 r1_zzxx = m_r[1].shuffle< 2, 2, 0, 0 >();
	Vector4 r1_ywwy = m_r[1].shuffle< 1, 3, 3, 1 >();
	Vector4 r1_yzwx = m_r[1].shuffle< 1, 2, 3, 0 >();
	Vector4 r1_zwxy = m_r[1].shuffle< 2, 3, 0, 1 >();
	Vector4 r1_wxyz = m_r[1].shuffle< 3, 0, 1, 2 >();

	Vector4 r2_zxxz = m_r[2].shuffle< 2, 0, 0, 2 >();
	Vector4 r2_wwyy = m_r[2].shuffle< 3, 3, 1, 1 >();
	Vector4 r2_yxwz = m_r[2].shuffle< 1, 0, 3, 2 >();
	Vector4 r2_wzyx = m_r[2].shuffle< 3, 2, 1, 0 >();
	Vector4 r2_zzxx = m_r[2].shuffle< 2, 2, 0, 0 >();
	Vector4 r2_ywwy = m_r[2].shuffle< 1, 3, 3, 1 >();
	Vector4 r2_yzwx = m_r[2].shuffle< 1, 2, 3, 0 >();
	Vector4 r2_zwxy = m_r[2].shuffle< 2, 3, 0, 1 >();
	Vector4 r2_wxyz = m_r[2].shuffle< 3, 0, 1, 2 >();

	Vector4 r3_zxxz = m_r[3].shuffle< 2, 0, 0, 2 >();
	Vector4 r3_wwyy = m_r[3].shuffle< 3, 3, 1, 1 >();
	Vector4 r3_yxwz = m_r[3].shuffle< 1, 0, 3, 2 >();
	Vector4 r3_wzyx = m_r[3].shuffle< 3, 2, 1, 0 >();
	Vector4 r3_zzxx = m_r[3].shuffle< 2, 2, 0, 0 >();
	Vector4 r3_ywwy = m_r[3].shuffle< 1, 3, 3, 1 >();
	Vector4 r3_yzwx = m_r[3].shuffle< 1, 2, 3, 0 >();
	Vector4 r3_zwxy = m_r[3].shuffle< 2, 3, 0, 1 >();
	Vector4 r3_wxyz = m_r[3].shuffle< 3, 0, 1, 2 >();

	Vector4 r_xxxx = r1_yzwx * (r2_zxxz * r3_wwyy - r2_wwyy * r3_zxxz) + r1_zwxy * (r2_wzyx * r3_yxwz - r2_yxwz * r3_wzyx) + r1_wxyz * (r2_ywwy * r3_zzxx - r2_zzxx * r3_ywwy);
	Vector4 r_yyyy = r2_yzwx * (r0_zxxz * r3_wwyy - r0_wwyy * r3_zxxz) + r2_zwxy * (r0_wzyx * r3_yxwz - r0_yxwz * r3_wzyx) + r2_wxyz * (r0_ywwy * r3_zzxx - r0_zzxx * r3_ywwy);
	Vector4 r_zzzz = r3_yzwx * (r0_zxxz * r1_wwyy - r0_wwyy * r1_zxxz) + r3_zwxy * (r0_wzyx * r1_yxwz - r0_yxwz * r1_wzyx) + r3_wxyz * (r0_ywwy * r1_zzxx - r0_zzxx * r1_ywwy);
	Vector4 r_wwww = r0_yzwx * (r2_zxxz * r1_wwyy - r2_wwyy * r1_zxxz) + r0_zwxy * (r2_wxyz * r1_yxwz - r2_yxwz * r1_wzyx) + r0_wxyz * (r2_ywwy * r1_zzxx - r2_zzxx * r1_ywwy);

	r_xxxx *= s;
	r_yyyy *= s;
	r_zzzz *= s;
	r_wwww *= s;

	return Matrix44(
		Vector4(r_xxxx.x(), r_yyyy.x(), r_zzzz.x(), r_wwww.x()),
		Vector4(r_xxxx.y(), r_yyyy.y(), r_zzzz.y(), r_wwww.y()),
		Vector4(r_xxxx.z(), r_yyyy.z(), r_zzzz.z(), r_wwww.z()),
		Vector4(r_xxxx.w(), r_yyyy.w(), r_zzzz.w(), r_wwww.w())
	);
}

T_MATH_INLINE Matrix44 Matrix44::inverseOrtho() const
{
	Scalar s = determinant();
	T_ASSERT (abs(s) > FUZZY_EPSILON);
	
	s = Scalar(1.0f) / s;

	return Matrix44(
		s * (m_r[1].y() * m_r[2].z() - m_r[1].z() * m_r[2].y()),
		s * (m_r[2].y() * m_r[0].z() - m_r[2].z() * m_r[0].y()),
		s * (m_r[0].y() * m_r[1].z() - m_r[0].z() * m_r[1].y()),
		0.0f,
		s * (m_r[1].z() * m_r[2].x() - m_r[1].x() * m_r[2].z()),
		s * (m_r[2].z() * m_r[0].x() - m_r[2].x() * m_r[0].z()),
		s * (m_r[0].z() * m_r[1].x() - m_r[0].x() * m_r[1].z()),
		0.0f,
		s * (m_r[1].x() * m_r[2].y() - m_r[1].y() * m_r[2].x()),
		s * (m_r[2].x() * m_r[0].y() - m_r[2].y() * m_r[0].x()),
		s * (m_r[0].x() * m_r[1].y() - m_r[0].y() * m_r[1].x()),
		0.0f,
		s * (m_r[1].x() * (m_r[2].z() * m_r[3].y() - m_r[2].y() * m_r[3].z()) + m_r[1].y() * (m_r[2].x() * m_r[3].z() - m_r[2].z() * m_r[3].x()) + m_r[1].z() * (m_r[2].y() * m_r[3].x() - m_r[2].x() * m_r[3].y())),
		s * (m_r[2].x() * (m_r[0].z() * m_r[3].y() - m_r[0].y() * m_r[3].z()) + m_r[2].y() * (m_r[0].x() * m_r[3].z() - m_r[0].z() * m_r[3].x()) + m_r[2].z() * (m_r[0].y() * m_r[3].x() - m_r[0].x() * m_r[3].y())),
		s * (m_r[3].x() * (m_r[0].z() * m_r[1].y() - m_r[0].y() * m_r[1].z()) + m_r[3].y() * (m_r[0].x() * m_r[1].z() - m_r[0].z() * m_r[1].x()) + m_r[3].z() * (m_r[0].y() * m_r[1].x() - m_r[0].x() * m_r[1].y())),
		1.0f
	);
}

T_MATH_INLINE void Matrix44::store(float* out) const
{
	m_r[0].store(&out[0]);
	m_r[1].store(&out[4]);
	m_r[2].store(&out[8]);
	m_r[3].store(&out[12]);
}

T_MATH_INLINE Vector4& Matrix44::operator () (int r)
{
	return m_r[r];
}

T_MATH_INLINE const Vector4& Matrix44::operator () (int r) const
{
	return m_r[r];
}

T_MATH_INLINE Scalar Matrix44::operator () (int r, int c) const
{
	return m_r[r][c];
}

T_MATH_INLINE Matrix44& Matrix44::operator = (const Matrix44& m)
{
	for (int i = 0; i < sizeof_array(m_r); ++i)
		this->m_r[i] = m.m_r[i];
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
	for (int r = 0; r < 4; ++r)
	{
		if (m_r[r] != m.m_r[r])
			return false;
	}
	return true;
}

T_MATH_INLINE bool Matrix44::operator != (const Matrix44& m) const
{
	for (int r = 0; r < 4; ++r)
	{
		if (m_r[r] != m.m_r[r])
			return true;
	}
	return false;
}

T_MATH_INLINE Matrix44 operator + (const Matrix44& lh, const Matrix44& rh)
{
	Matrix44 m;
	for (int r = 0; r < 4; ++r)
		m.m_r[r] = lh.m_r[r] + rh.m_r[r];
	return m;
}

T_MATH_INLINE Matrix44 operator - (const Matrix44& lh, const Matrix44& rh)
{
	Matrix44 m;
	for (int r = 0; r < 4; ++r)
		m.m_r[r] = lh.m_r[r] - rh.m_r[r];
	return m;
}

T_MATH_INLINE Vector4 operator * (const Matrix44& m, const Vector4& v)
{
	return v.x() * m.m_r[0] + v.y() * m.m_r[1] + v.z() * m.m_r[2] + v.w() * m.m_r[3];
}

T_MATH_INLINE Matrix44 operator * (const Matrix44& lh, const Matrix44& rh)
{
	Vector4 lr0_xxxx = lh.m_r[0].shuffle< 0, 0, 0, 0 >();
	Vector4 lr0_yyyy = lh.m_r[0].shuffle< 1, 1, 1, 1 >();
	Vector4 lr0_zzzz = lh.m_r[0].shuffle< 2, 2, 2, 2 >();
	Vector4 lr0_wwww = lh.m_r[0].shuffle< 3, 3, 3, 3 >();

	Vector4 lr1_xxxx = lh.m_r[1].shuffle< 0, 0, 0, 0 >();
	Vector4 lr1_yyyy = lh.m_r[1].shuffle< 1, 1, 1, 1 >();
	Vector4 lr1_zzzz = lh.m_r[1].shuffle< 2, 2, 2, 2 >();
	Vector4 lr1_wwww = lh.m_r[1].shuffle< 3, 3, 3, 3 >();

	Vector4 lr2_xxxx = lh.m_r[2].shuffle< 0, 0, 0, 0 >();
	Vector4 lr2_yyyy = lh.m_r[2].shuffle< 1, 1, 1, 1 >();
	Vector4 lr2_zzzz = lh.m_r[2].shuffle< 2, 2, 2, 2 >();
	Vector4 lr2_wwww = lh.m_r[2].shuffle< 3, 3, 3, 3 >();

	Vector4 lr3_xxxx = lh.m_r[3].shuffle< 0, 0, 0, 0 >();
	Vector4 lr3_yyyy = lh.m_r[3].shuffle< 1, 1, 1, 1 >();
	Vector4 lr3_zzzz = lh.m_r[3].shuffle< 2, 2, 2, 2 >();
	Vector4 lr3_wwww = lh.m_r[3].shuffle< 3, 3, 3, 3 >();

	return Matrix44(
		lr0_xxxx * rh.m_r[0] + lr0_yyyy * rh.m_r[1] + lr0_zzzz * rh.m_r[2] + lr0_wwww * rh.m_r[3],
		lr1_xxxx * rh.m_r[0] + lr1_yyyy * rh.m_r[1] + lr1_zzzz * rh.m_r[2] + lr1_wwww * rh.m_r[3],
		lr2_xxxx * rh.m_r[0] + lr2_yyyy * rh.m_r[1] + lr2_zzzz * rh.m_r[2] + lr2_wwww * rh.m_r[3],
		lr3_xxxx * rh.m_r[0] + lr3_yyyy * rh.m_r[1] + lr3_zzzz * rh.m_r[2] + lr3_wwww * rh.m_r[3]
	);
}

T_MATH_INLINE Matrix44 orthoLh(float width, float height, float zn, float zf)
{
	return Matrix44(
		2 / width, 0,          0,               0,
		0,         2 / height, 0,               0,
		0,         0,          1 / (zf - zn),   0,
		0,         0,          -zn / (zf - zn), 1
	);
}

T_MATH_INLINE Matrix44 orthoLh(float left, float top, float right, float bottom, float zn, float zf)
{
	return Matrix44(
		2 / (right - left),              0,                               0,               0,
		0,                               2 / (bottom - top),              0,               0,
		0,                               0,                               1 / (zf - zn),   0,
		(left + right) / (left - right), (bottom + top) / (bottom - top), -zn / (zf - zn), 1
	);
}

T_MATH_INLINE Matrix44 orthoRh(float width, float height, float zn, float zf)
{
	return Matrix44(
		2 / width, 0,          0,              0,
		0,         2 / height, 0,              0,
		0,         0,          1 / (zn - zf),  0,
		0,         0,          zn / (zf - zn), 1
	);
}

T_MATH_INLINE Matrix44 perspectiveLh(float fov, float aspect, float zn, float zf)
{
	float h = cosf(fov / 2.0f) / sinf(fov / 2.0f);
	float w = h / aspect;

	return Matrix44(
		w, 0, 0,			0,
		0, h, 0,			0,
		0, 0, zf / (zf - zn),		1,
		0, 0, -zn * zf / (zf - zn),	0
	);
}

T_MATH_INLINE Matrix44 perspectiveRh(float fov, float aspect, float zn, float zf)
{
	float h = cosf(fov / 2.0f) / sinf(fov / 2.0f);
	float w = h / aspect;

	return Matrix44(
		   w, 0.0f,                 0.0f,  0.0f,
		0.0f,    h,                 0.0f,  0.0f,
		0.0f, 0.0f,       zf / (zn - zf), -1.0f,
		0.0f, 0.0f, -zn * zf / (zn - zf),  0.0f
	);
}

T_MATH_INLINE Matrix44 translate(const Vector4& t)
{
	return Matrix44(
		 1.0f,  0.0f,  0.0f, 0.0f, 
		 0.0f,  1.0f,  0.0f, 0.0f,
		 0.0f,  0.0f,  1.0f, 0.0f,
		t.x(), t.y(), t.z(), 1.0f
	);
}

T_MATH_INLINE Matrix44 translate(float x, float y, float z)
{
	return Matrix44(
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		   x,    y,    z, 1.0f
	);
}

T_MATH_INLINE Matrix44 rotateX(float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);
	return Matrix44(
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f,    c,    s, 0.0f,
		0.0f,   -s,    c, 0.0f,
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
		   c,    s, 0.0f, 0.0f, 
		  -s,    c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix44 rotate(const Vector4& axis, float angle)
{
	Scalar c = Scalar(cosf(angle));
	Scalar s = Scalar(sinf(angle));
	Scalar t = Scalar(1.0f) - c;
	return Matrix44(
		t * axis.x() * axis.x() + c,
		t * axis.x() * axis.y() + s * axis.z(),
		t * axis.x() * axis.z() - s * axis.y(),
		0.0f,
		t * axis.x() * axis.y() - s * axis.z(),
		t * axis.y() * axis.y() + c,
		t * axis.y() * axis.z() + s * axis.x(),
		0.0f,
		t * axis.x() * axis.z() + s * axis.y(),
		t * axis.y() * axis.z() - s * axis.x(),
		t * axis.z() * axis.z() + c,
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
		x.x(), y.x(), z.x(), 0.0f,
		x.y(), y.y(), z.y(), 0.0f,
		x.z(), y.z(), z.z(), 0.0f,
		-dot3(x, position), -dot3(y, position), -dot3(z, position), 1.0f
	);
}

}

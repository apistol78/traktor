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
	for (int i = 0; i < 16; ++i)
		this->m[i] = m.m[i];
}

T_MATH_INLINE Matrix44::Matrix44(const Vector4& axisX, const Vector4& axisY, const Vector4& axisZ, const Vector4& translation)
:	e11(axisX.x()), e12(axisX.y()), e13(axisX.z()), e14(axisX.w())
,	e21(axisY.x()), e22(axisY.y()), e23(axisY.z()), e24(axisY.w())
,	e31(axisZ.x()), e32(axisZ.y()), e33(axisZ.z()), e34(axisZ.w())
,	e41(translation.x()), e42(translation.y()), e43(translation.z()), e44(translation.w())	
{
}

T_MATH_INLINE Matrix44::Matrix44(float e11_, float e12_, float e13_, float e14_, float e21_, float e22_, float e23_, float e24_, float e31_, float e32_, float e33_, float e34_, float e41_, float e42_, float e43_, float e44_)
:	e11(e11_), e12(e12_), e13(e13_), e14(e14_)
,	e21(e21_), e22(e22_), e23(e23_), e24(e24_)
,	e31(e31_), e32(e32_), e33(e33_), e34(e34_)
,	e41(e41_), e42(e42_), e43(e43_), e44(e44_)
{
}

T_MATH_INLINE Matrix44::Matrix44(const float* p)
{
	for (int i = 0; i < 16; ++i)
		m[i] = p[i];
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
	return Vector4(e11, e12, e13, e14);
}

T_MATH_INLINE Vector4 Matrix44::axisY() const
{
	return Vector4(e21, e22, e23, e24);
}

T_MATH_INLINE Vector4 Matrix44::axisZ() const
{
	return Vector4(e31, e32, e33, e34);
}

T_MATH_INLINE Vector4 Matrix44::translation() const
{
	return Vector4(e41, e42, e43, e44);
}

T_MATH_INLINE Vector4 Matrix44::diagonal() const
{
	return Vector4(e11, e22, e33, e44);
}

T_MATH_INLINE bool Matrix44::isOrtho() const
{
	return std::fabs(determinant()) == 1.0f;
}

T_MATH_INLINE float Matrix44::determinant() const
{
	return
		  (e11 * e22 - e12 * e21) * (e33 * e44 - e34 * e43)
		- (e11 * e23 - e13 * e21) * (e32 * e44 - e34 * e42)
		+ (e11 * e24 - e14 * e21) * (e32 * e43 - e33 * e42)
		+ (e12 * e23 - e13 * e22) * (e31 * e44 - e34 * e41)
		- (e12 * e24 - e14 * e22) * (e31 * e43 - e33 * e41)
		+ (e13 * e24 - e14 * e23) * (e31 * e42 - e32 * e41);
}

T_MATH_INLINE Matrix44 Matrix44::transpose() const
{
	return Matrix44(
		e11, e21, e31, e41,
		e12, e22, e32, e42,
		e13, e23, e33, e43,
		e14, e24, e34, e44
	);
}

T_MATH_INLINE Matrix44 Matrix44::inverse() const
{
	Matrix44 r;

	float s = determinant();
	if (s == 0.0f) return identity();

	s = 1 / s;
	r.e11 = s * (e22 * (e33 * e44 - e34 * e43) + e23 * (e34 * e42 - e32 * e44) + e24 * (e32 * e43 - e33 * e42));
	r.e12 = s * (e32 * (e13 * e44 - e14 * e43) + e33 * (e14 * e42 - e12 * e44) + e34 * (e12 * e43 - e13 * e42));
	r.e13 = s * (e42 * (e13 * e24 - e14 * e23) + e43 * (e14 * e22 - e12 * e24) + e44 * (e12 * e23 - e13 * e22));
	r.e14 = s * (e12 * (e24 * e33 - e23 * e34) + e13 * (e22 * e34 - e24 * e32) + e14 * (e23 * e32 - e22 * e33));
	r.e21 = s * (e23 * (e31 * e44 - e34 * e41) + e24 * (e33 * e41 - e31 * e43) + e21 * (e34 * e43 - e33 * e44));
	r.e22 = s * (e33 * (e11 * e44 - e14 * e41) + e34 * (e13 * e41 - e11 * e43) + e31 * (e14 * e43 - e13 * e44));
	r.e23 = s * (e43 * (e11 * e24 - e14 * e21) + e44 * (e13 * e21 - e11 * e23) + e41 * (e14 * e23 - e13 * e24));
	r.e24 = s * (e13 * (e24 * e31 - e21 * e34) + e14 * (e21 * e33 - e23 * e31) + e11 * (e23 * e34 - e24 * e33));
	r.e31 = s * (e24 * (e31 * e42 - e32 * e41) + e21 * (e32 * e44 - e34 * e42) + e22 * (e34 * e41 - e31 * e44));
	r.e32 = s * (e34 * (e11 * e42 - e12 * e41) + e31 * (e12 * e44 - e14 * e42) + e32 * (e14 * e41 - e11 * e44));
	r.e33 = s * (e44 * (e11 * e22 - e12 * e21) + e41 * (e12 * e24 - e14 * e22) + e42 * (e14 * e21 - e11 * e24));
	r.e34 = s * (e14 * (e22 * e31 - e21 * e32) + e11 * (e24 * e32 - e22 * e34) + e12 * (e21 * e34 - e24 * e31));
	r.e41 = s * (e21 * (e33 * e42 - e32 * e43) + e22 * (e31 * e43 - e33 * e41) + e23 * (e32 * e41 - e31 * e42));
	r.e42 = s * (e31 * (e13 * e42 - e12 * e43) + e32 * (e11 * e43 - e13 * e41) + e33 * (e12 * e41 - e11 * e42));
	r.e43 = s * (e41 * (e13 * e22 - e12 * e23) + e42 * (e11 * e23 - e13 * e21) + e43 * (e12 * e21 - e11 * e22));
	r.e44 = s * (e11 * (e22 * e33 - e23 * e32) + e12 * (e23 * e31 - e21 * e33) + e13 * (e21 * e32 - e22 * e31));

	return r;
}

T_MATH_INLINE Matrix44 Matrix44::inverseOrtho() const
{
	float s = determinant();
	T_ASSERT (abs(s) > FUZZY_EPSILON);
	
	s = 1.0f / s;
	
	return Matrix44(
		s * ((e22 * e33) - (e23 * e32)),
		s * ((e32 * e13) - (e33 * e12)),
		s * ((e12 * e23) - (e13 * e22)),
		0.0f,
		s * ((e23 * e31) - (e21 * e33)),
		s * ((e33 * e11) - (e31 * e13)),
		s * ((e13 * e21) - (e11 * e23)),
		0.0f,
		s * ((e21 * e32) - (e22 * e31)),
		s * ((e31 * e12) - (e32 * e11)),
		s * ((e11 * e22) - (e12 * e21)),
		0.0f,
		s * (e21 * (e33 * e42 - e32 * e43) + e22 * (e31 * e43 - e33 * e41) + e23 * (e32 * e41 - e31 * e42)),
		s * (e31 * (e13 * e42 - e12 * e43) + e32 * (e11 * e43 - e13 * e41) + e33 * (e12 * e41 - e11 * e42)),
		s * (e41 * (e13 * e22 - e12 * e23) + e42 * (e11 * e23 - e13 * e21) + e43 * (e12 * e21 - e11 * e22)),
		1.0f
	);
}

T_MATH_INLINE Matrix44& Matrix44::operator = (const Matrix44& m)
{
	for (int i = 0; i < 16; ++i)
		this->m[i] = m.m[i];
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
		for (int c = 0; c < 4; ++c)
		{
			if (abs(e[r][c] - m.e[r][c]) > FUZZY_EPSILON)
				return false;
		}
	}
	return true;
}

T_MATH_INLINE bool Matrix44::operator != (const Matrix44& m) const
{
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			if (abs(e[r][c] - m.e[r][c]) > FUZZY_EPSILON)
				return true;
		}
	}
	return false;
}

T_MATH_INLINE Matrix44 operator + (const Matrix44& lh, const Matrix44& rh)
{
	Matrix44 m;
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
			m.e[r][c] = lh.e[r][c] + rh.e[r][c];
	return m;
}

T_MATH_INLINE Matrix44 operator - (const Matrix44& lh, const Matrix44& rh)
{
	Matrix44 m;
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
			m.e[r][c] = lh.e[r][c] - rh.e[r][c];
	return m;
}

T_MATH_INLINE Vector4 operator * (const Matrix44& m, const Vector4& v)
{
	return v.x() * m.axisX() + v.y() * m.axisY() + v.z() * m.axisZ() + v.w() * m.translation();
}

T_MATH_INLINE Matrix44 operator * (const Matrix44& lh, const Matrix44& rh)
{
	return Matrix44(
		lh.e11 * rh.e11 + lh.e12 * rh.e21 + lh.e13 * rh.e31 + lh.e14 * rh.e41,
		lh.e11 * rh.e12 + lh.e12 * rh.e22 + lh.e13 * rh.e32 + lh.e14 * rh.e42,
		lh.e11 * rh.e13 + lh.e12 * rh.e23 + lh.e13 * rh.e33 + lh.e14 * rh.e43,
		lh.e11 * rh.e14 + lh.e12 * rh.e24 + lh.e13 * rh.e34 + lh.e14 * rh.e44,
		lh.e21 * rh.e11 + lh.e22 * rh.e21 + lh.e23 * rh.e31 + lh.e24 * rh.e41,
		lh.e21 * rh.e12 + lh.e22 * rh.e22 + lh.e23 * rh.e32 + lh.e24 * rh.e42,
		lh.e21 * rh.e13 + lh.e22 * rh.e23 + lh.e23 * rh.e33 + lh.e24 * rh.e43,
		lh.e21 * rh.e14 + lh.e22 * rh.e24 + lh.e23 * rh.e34 + lh.e24 * rh.e44,
		lh.e31 * rh.e11 + lh.e32 * rh.e21 + lh.e33 * rh.e31 + lh.e34 * rh.e41,
		lh.e31 * rh.e12 + lh.e32 * rh.e22 + lh.e33 * rh.e32 + lh.e34 * rh.e42,
		lh.e31 * rh.e13 + lh.e32 * rh.e23 + lh.e33 * rh.e33 + lh.e34 * rh.e43,
		lh.e31 * rh.e14 + lh.e32 * rh.e24 + lh.e33 * rh.e34 + lh.e34 * rh.e44,
		lh.e41 * rh.e11 + lh.e42 * rh.e21 + lh.e43 * rh.e31 + lh.e44 * rh.e41,
		lh.e41 * rh.e12 + lh.e42 * rh.e22 + lh.e43 * rh.e32 + lh.e44 * rh.e42,
		lh.e41 * rh.e13 + lh.e42 * rh.e23 + lh.e43 * rh.e33 + lh.e44 * rh.e43,
		lh.e41 * rh.e14 + lh.e42 * rh.e24 + lh.e43 * rh.e34 + lh.e44 * rh.e44
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

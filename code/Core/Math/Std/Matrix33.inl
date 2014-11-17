#include <cmath>
#include "Core/Math/Matrix33.h"
#include "Core/Math/Const.h"

namespace traktor
{

T_MATH_INLINE Matrix33::Matrix33()
{
}

T_MATH_INLINE Matrix33::Matrix33(const Matrix33& m)
{
	for (int i = 0; i < 9; ++i)
		this->m[i] = m.m[i];
}

T_MATH_INLINE Matrix33::Matrix33(const Vector4& r1, const Vector4& r2, const Vector4& r3)
{
	e11 = r1.x(); e12 = r1.y(); e13 = r1.z();
	e21 = r2.x(); e22 = r2.y(); e23 = r2.z();
	e31 = r3.x(); e32 = r3.y(); e33 = r3.z();
}

T_MATH_INLINE Matrix33::Matrix33(float e11_, float e12_, float e13_, float e21_, float e22_, float e23_, float e31_, float e32_, float e33_)
:	e11(e11_), e12(e12_), e13(e13_)
,	e21(e21_), e22(e22_), e23(e23_)
,	e31(e31_), e32(e32_), e33(e33_)
{
}

T_MATH_INLINE Matrix33::Matrix33(const float m_[9])
:	e11(m_[0]), e12(m_[1]), e13(m_[2])
,	e21(m_[3]), e22(m_[4]), e23(m_[5])
,	e31(m_[6]), e32(m_[7]), e33(m_[8])
{
}

T_MATH_INLINE const Matrix33& Matrix33::zero()
{
	static Matrix33 identity(
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f
	);
	return identity;
}

T_MATH_INLINE const Matrix33& Matrix33::identity()
{
	static Matrix33 identity(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	);
	return identity;
}

T_MATH_INLINE Vector4 Matrix33::diagonal() const
{
	return Vector4(e11, e22, e33, 0.0f);
}

T_MATH_INLINE float Matrix33::determinant() const
{
	return
		e11 * (e22 * e33 - e32 * e23) -
		e12 * (e21 * e33 - e31 * e23) +
		e13 * (e21 * e32 - e31 * e22);
}

T_MATH_INLINE Matrix33 Matrix33::transpose() const
{
	return Matrix33(
		e11, e21, e31,
		e12, e22, e32,
		e13, e23, e33
	);
}

T_MATH_INLINE Matrix33 Matrix33::inverse() const
{
	float s = determinant();
	if (s == 0)
		s = 1;

	s = 1.0f / s;
	return Matrix33(
		s *  (e22 * e33 - e23 * e32),
		s * -(e12 * e33 - e13 * e32),
		s *  (e12 * e23 - e13 * e22),
		s * -(e21 * e33 - e23 * e31),
		s *  (e11 * e33 - e13 * e31),
		s * -(e11 * e23 - e13 * e21),
		s *  (e21 * e32 - e22 * e31),
		s * -(e11 * e32 - e12 * e31),
		s *  (e11 * e22 - e12 * e21)
	);
}

T_MATH_INLINE Matrix33& Matrix33::operator = (const Matrix33& m_)
{
	for (int i = 0; i < 9; ++i)
		this->m[i] = m_.m[i];
	return *this;
}

T_MATH_INLINE Matrix33& Matrix33::operator *= (const Matrix33& m_)
{
	*this = *this * m_;
	return *this;
}

T_MATH_INLINE Vector2 operator * (const Matrix33& m, const Vector2& v)
{
	return Vector2(
		v.x * m.e11 + v.y * m.e12 + m.e13,
		v.x * m.e21 + v.y * m.e22 + m.e23
	);
}

T_MATH_INLINE Vector4 operator * (const Matrix33& m, const Vector4& v)
{
	return Vector4(
		v.x() * m.e11 + v.y() * m.e12 + v.z() * m.e13,
		v.x() * m.e21 + v.y() * m.e22 + v.z() * m.e23,
		v.x() * m.e31 + v.y() * m.e32 + v.z() * m.e33,
		0.0f
	);
}

T_MATH_INLINE Matrix33 operator * (const Matrix33& lh, const Matrix33& rh)
{
	Matrix33 m;
	for (int c = 0; c < 3; ++c)
	{
		for (int r = 0; r < 3; ++r)
		{
			m.e[r][c] =
				lh.e[r][0] * rh.e[0][c] +
				lh.e[r][1] * rh.e[1][c] +
				lh.e[r][2] * rh.e[2][c];
		}
	}
	return m;
}

T_MATH_INLINE Matrix33 translate(float x, float y)
{
	return Matrix33(
		1.0f, 0.0f, x,
		0.0f, 1.0f, y,
		0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix33 rotate(float angle)
{
	float c = float(std::cos(angle));
	float s = float(std::sin(angle));
	return Matrix33(
		    c,   -s, 0.0f,
		    s,    c, 0.0f,
		 0.0f, 0.0f, 1.0f
	);
}

T_MATH_INLINE Matrix33 scale(float x, float y)
{
	return Matrix33(
		x, 0.0f, 0.0f,
		0.0f, y, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

}

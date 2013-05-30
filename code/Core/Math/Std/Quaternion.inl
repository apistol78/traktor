#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Quaternion.h"

namespace traktor
{

T_MATH_INLINE Quaternion::Quaternion()
{
}

T_MATH_INLINE Quaternion::Quaternion(const Quaternion& q)
:	e(q.e)
{
}

T_MATH_INLINE Quaternion::Quaternion(float x, float y, float z, float w)
:	e(x, y, z, w)
{
}

T_MATH_INLINE Quaternion::Quaternion(const Matrix44& m)
{
	Vector4 mc0 = m.get(0);
	Vector4 mc1 = m.get(1);
	Vector4 mc2 = m.get(2);

	Scalar trace = Scalar(1.0f) + mc0.x() + mc1.y() + mc2.z();
	if (trace > FUZZY_EPSILON)
	{
		Scalar S = squareRoot(trace) * Scalar(2.0f);
		Scalar iS = Scalar(1.0f) / S;
		e.set(
			(mc1.z() - mc2.y()) * iS,
			(mc2.x() - mc0.z()) * iS,
			(mc0.y() - mc1.x()) * iS,
			Scalar(0.25f) * S
		);
	}
	else
	{
		if (mc0.x() > mc1.y() && mc0.x() > mc2.z())
		{
			Scalar S = squareRoot(Scalar(1.0f) + mc0.x() - mc1.y() - mc2.z()) * Scalar(2.0f);
			Scalar iS = Scalar(1.0f) / S;
			e.set(
				Scalar(0.25f) * S,
				(mc0.y() + mc1.x()) * iS,
				(mc2.x() + mc0.z()) * iS,
				(mc1.z() - mc2.y()) * iS
			);
		}
		else if (m(1, 1) > m(2, 2))
		{
			Scalar S = squareRoot(Scalar(1.0f) + mc1.y() - mc0.x() - mc2.z()) * Scalar(2.0f);
			Scalar iS = Scalar(1.0f) / S;
			e.set(
				(mc0.y() + mc1.x()) * iS,
				Scalar(0.25f) * S,
				(mc1.z() + mc2.y()) * iS,
				(mc2.x() - mc0.z()) * iS
			);
		}
		else
		{
			Scalar S = squareRoot(Scalar(1.0f) + mc2.z() - mc0.x() - mc1.y()) * Scalar(2.0f);
			Scalar iS = Scalar(1.0f) / S;
			e.set(
				(mc2.x() + mc0.z()) * iS,
				(mc1.z() + mc2.y()) * iS,
				Scalar(0.25f) * S,
				(mc0.y() - mc1.x()) * iS
			);
		}
	}
}

T_MATH_INLINE Quaternion::Quaternion(const Vector4& from, const Vector4& to)
{
	T_ASSERT (abs(from.w()) == 0.0f);
	T_ASSERT (abs(to.w()) == 0.0f);

	Vector4 c = cross(from, to);
	Scalar d = dot3(from, to);

	if (d < -1.0f + FUZZY_EPSILON)
	{
		e = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		Scalar s = squareRoot((Scalar(1.0f) + d) * Scalar(2.0f));
		Scalar rs = Scalar(1.0f) / s;
		e = (c.xyz0() * rs) + Vector4(0.0f, 0.0f, 0.0f, s * Scalar(0.5f));
	}
}

T_MATH_INLINE Quaternion::Quaternion(const Vector4& _e)
:	e(_e)
{
}

T_MATH_INLINE Quaternion Quaternion::identity()
{
	return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

T_MATH_INLINE Quaternion Quaternion::normalized() const
{
	Scalar ln = e.length();
	if (ln > FUZZY_EPSILON)
		return Quaternion(e / ln);
	else
		return identity();
}

T_MATH_INLINE Quaternion Quaternion::inverse() const
{
	return Quaternion(e * Vector4(-1.0f, -1.0f, -1.0f, 1.0f));
}

T_MATH_INLINE Vector4 Quaternion::toAxisAngle() const
{
	Scalar ln = dot3(e, e);
	if (abs(ln) >= FUZZY_EPSILON)
	{
		Scalar scale = reciprocalSquareRoot(ln);
		Scalar angle = Scalar(2.0f * acosf(e.w()));
		return Vector4(e * scale * angle).xyz0();
	}
	else
		return Vector4::zero();
}

T_MATH_INLINE Quaternion Quaternion::fromAxisAngle(const Vector4& axisAngle)
{
	Scalar angle = axisAngle.length();
	if (abs(angle) >= FUZZY_EPSILON)
	{
		Vector4 axis = axisAngle / angle;
		Scalar half = angle / Scalar(2.0f);
		Scalar sh(sinf(half));
		Scalar ch(cosf(half));
		return Quaternion(axis.xyz0() * sh + Vector4(0.0f, 0.0f, 0.0f, ch));
	}
	else
		return Quaternion::identity();
}

T_MATH_INLINE Quaternion Quaternion::fromAxisAngle(const Vector4& axis, float angle)
{
	float half = angle / 2.0f;
	Scalar sh(sinf(half));
	Scalar ch(cosf(half));
	return Quaternion(axis.xyz0() * sh + Vector4(0.0f, 0.0f, 0.0f, ch));
}

T_MATH_INLINE Matrix44 Quaternion::toMatrix44() const
{
	float T_MATH_ALIGN16 s[4];
	e.storeAligned(s);

	const float& x = s[0];
	const float& y = s[1];
	const float& z = s[2];
	const float& w = s[3];

	return Matrix44(
		1.0f - 2.0f * y * y - 2.0f * z * z,
		2.0f * x * y - 2.0f * w * z,
		2.0f * x * z + 2.0f * w * y,
		0.0f,

		2.0f * x * y + 2.0f * w * z,
		1.0f - 2.0f * x * x - 2.0f * z * z,
		2.0f * y * z - 2.0f * w * x,
		0.0f,

		2.0f * x * z - 2.0f * w * y,
		2.0f * y * z + 2.0f * w * x,
		1.0f - 2.0f * x * x - 2.0f * y * y,
		0.0f,

		0.0f,
		0.0f,
		0.0f,
		1.0f
	);
}

T_MATH_INLINE Vector4 Quaternion::toEulerAngles() const
{
	float h, p, b;
	toEulerAngles(h, p, b);
	return Vector4(h, p, b, 0.0f);
}

T_MATH_INLINE void Quaternion::toEulerAngles(float& outHead, float& outPitch, float& outBank) const
{
	const Vector4 axisX(1.0f, 0.0f, 0.0f, 0.0f);
	const Vector4 axisZ(0.0f, 0.0f, 1.0f, 0.0f);

	Vector4 axisX_2 = (*this) * axisX;
	Vector4 axisZ_2 = (*this) * axisZ;

	outHead = atan2f(
		axisZ_2.x(),
		axisZ_2.z()
	);
	T_ASSERT (!isNan(outHead));

	Quaternion Qt0 = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -outHead);
	Vector4 axisX_3 = Qt0 * axisX_2;
	Vector4 axisZ_3 = Qt0 * axisZ_2;

	outPitch = -atan2f(
		axisZ_3.y(),
		axisZ_3.z()
	);
	T_ASSERT (!isNan(outPitch));

	Quaternion Qt1 = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), -outPitch);
	Vector4 axisX_4 = Qt1 * axisX_3;

	outBank = atan2f(
		axisX_4.y(),
		axisX_4.x()
	);
	T_ASSERT (!isNan(outBank));
}

T_MATH_INLINE Quaternion Quaternion::fromEulerAngles(float head, float pitch, float bank)
{
	Quaternion Qh = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), head);
	Quaternion Qp = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), pitch);
	Quaternion Qb = Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f, 0.0f), bank);
	return Qh * Qp * Qb;
}

T_MATH_INLINE Quaternion Quaternion::fromEulerAngles(const Vector4& angles)
{
	Quaternion Qh = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), angles.x());
	Quaternion Qp = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), angles.y());
	Quaternion Qb = Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f, 0.0f), angles.z());
	return Qh * Qp * Qb;
}

T_MATH_INLINE Quaternion& Quaternion::operator *= (const Quaternion& r)
{
	*this = *this * r;
	return *this;
}

T_MATH_INLINE bool Quaternion::operator == (const Quaternion& v) const
{
	return v.e == e;
}

T_MATH_INLINE bool Quaternion::operator != (const Quaternion& v) const
{
	return v.e != e;
}

T_MATH_INLINE Quaternion operator + (const Quaternion& l, const Quaternion& r)
{
	return Quaternion(l.e + r.e);
}

T_MATH_INLINE Quaternion operator * (const Quaternion& l, float r)
{
	return Quaternion(l.e * Scalar(r));
}

T_MATH_INLINE Quaternion operator * (const Quaternion& l, const Quaternion& r)
{
	const static Vector4 pppn(1.0f, 1.0f, 1.0f, -1.0f);

	Vector4 l_wwww = l.e.shuffle< 3, 3, 3, 3 >();
	Vector4 r_wwwx = r.e.shuffle< 3, 3, 3, 0 >();
	Vector4 l_xyzx = l.e.shuffle< 0, 1, 2, 0 >();
	Vector4 l_yzxy = l.e.shuffle< 1, 2, 0, 1 >();
	Vector4 r_zxyy = r.e.shuffle< 2, 0, 1, 1 >();
	Vector4 l_zxyz = l.e.shuffle< 2, 0, 1, 2 >();
	Vector4 r_yzxz = r.e.shuffle< 1, 2, 0, 2 >();

	return Quaternion(
		l_wwww * r.e +
		l_xyzx * r_wwwx * pppn +
		l_yzxy * r_zxyy * pppn -
		l_zxyz * r_yzxz
	);
}

T_MATH_INLINE Vector4 operator * (const Quaternion& q, const Vector4& v)
{
	Quaternion qv(v.xyz0());
	Quaternion qvp = q * qv * q.inverse();
	return qvp.e.xyz0() + v * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

T_MATH_INLINE Quaternion lerp(const Quaternion& a, const Quaternion& b, float c)
{
	return slerp(a, b, c);
}

T_MATH_INLINE Quaternion slerp(const Quaternion& a, const Quaternion& b, float c)
{
	Scalar scale1;
	Scalar scale2;

	Quaternion A = a.normalized();
	Quaternion B = b.normalized();

	Scalar phi = dot4(A.e, B.e);
	if (phi < 0.0f)
	{
		phi = -phi;
		B.e = -B.e;
	}

	if (phi > Scalar(0.95f))
		return Quaternion(lerp(A.e, B.e, Scalar(c))).normalized();

	float theta_0 = acosf(phi);
	return (A * sinf(theta_0 * (1 - c)) + B * sinf(theta_0 * c)) * (1.0f / sinf(theta_0));
}

}

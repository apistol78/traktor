#include <limits>
#include "Core/Math/Quaternion.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Const.h"
#if defined(_DEBUG)
#include "Core/Math/Float.h"
#endif

namespace traktor
{

#if defined(_DEBUG)
#define VALIDATE(v) \
	T_ASSERT (!isNan((v).x)); \
	T_ASSERT (!isNan((v).y)); \
	T_ASSERT (!isNan((v).z)); \
	T_ASSERT (!isNan((v).w));

#else
#define VALIDATE(v)
#endif

T_MATH_INLINE Quaternion::Quaternion()
#if defined(_DEBUG)
:	x(std::numeric_limits< float >::signaling_NaN())
,	y(std::numeric_limits< float >::signaling_NaN())
,	z(std::numeric_limits< float >::signaling_NaN())
,	w(std::numeric_limits< float >::signaling_NaN())
#endif
{
}

T_MATH_INLINE Quaternion::Quaternion(const Quaternion& q)
:	x(q.x)
,	y(q.y)
,	z(q.z)
,	w(q.w)
{
	VALIDATE(*this);
}

T_MATH_INLINE Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
	VALIDATE(*this);
}

T_MATH_INLINE Quaternion::Quaternion(const Vector4& axisAngle)
{
	T_ASSERT (axisAngle.w() == 0.0f);

	float angle = axisAngle.length();

	if (abs(angle) >= FUZZY_EPSILON)
	{
		float half = angle / 2.0f;
		x = axisAngle.x() * sinf(half) / angle;
		y = axisAngle.y() * sinf(half) / angle;
		z = axisAngle.z() * sinf(half) / angle;
		w = cosf(half);
	}
	else
	{
		x =
		y =
		z = 0.0f;
		w = 1.0f;
	}

	VALIDATE(*this);
}

T_MATH_INLINE Quaternion::Quaternion(const Matrix44& m)
{
	float trace = 1.0f + m(0, 0) + m(1, 1) + m(2, 2);
	if (trace > FUZZY_EPSILON)
	{
		float S = sqrtf(trace) * 2;
		x = (m(2, 1) - m(1, 2)) / S;
		y = (m(0, 2) - m(2, 0)) / S;
		z = (m(1, 0) - m(0, 1)) / S;
		w = 0.25f * S;
	}
	else
	{
		if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2))
		{
			float S = sqrtf(1.0f + m(0, 0) - m(1, 1) - m(2, 2)) * 2;
			x = 0.25f * S;
			y = (m(1, 0) + m(0, 1)) / S;
			z = (m(0, 2) + m(2, 0)) / S;
			w = (m(2, 1) - m(1, 2)) / S;
		}
		else if (m(1, 1) > m(2, 2))
		{
			float S = sqrtf(1.0f + m(1, 1) - m(0, 0) - m(2, 2)) * 2;
			x = (m(1, 0) + m(0, 1)) / S;
			y = 0.25f * S;
			z = (m(2, 1) + m(1, 2)) / S;
			w = (m(0, 2) - m(2, 0)) / S;
		}
		else
		{
			float S = sqrtf(1.0f + m(2, 2) - m(0, 0) - m(1, 1)) * 2;
			x = (m(0, 2) + m(2, 0)) / S;
			y = (m(2, 1) + m(1, 2)) / S;
			z = 0.25f * S;
			w = (m(1, 0) - m(0, 1)) / S;
		}
	}
	VALIDATE(*this);
}

T_MATH_INLINE Quaternion::Quaternion(float head, float pitch, float bank)
{
	float c1 = cosf(head / 2.0f);
	float c2 = cosf(pitch / 2.0f);
	float c3 = cosf(bank / 2.0f);
	float s1 = sinf(head / 2.0f);
	float s2 = sinf(pitch / 2.0f);
	float s3 = sinf(bank / 2.0f);

	x = s1 * s2 * c3 + c1 * c2 * s3;
	y = s1 * c2 * c3 + c1 * s2 * s3;
	z = c1 * s2 * c3 - s1 * c2 * s3;
	w = c1 * c2 * c3 - s1 * s2 * s3;

	VALIDATE(*this);
}

T_MATH_INLINE Quaternion::Quaternion(const Vector4& from, const Vector4& to)
{
	T_ASSERT (abs(from.w()) == 0.0f);
	T_ASSERT (abs(to.w()) == 0.0f);

	Vector4 c = cross(from, to);
	float d = dot3(from, to);

	if (d < -1.0f + FUZZY_EPSILON)
	{
		x =
		y =
		z = 0.0f;
		w = 1.0f;
	}
	else
	{
		float s = sqrtf((1.0f + d) * 2.0f);
		float rs = 1.0f / s;
		x = c.x() * rs;
		y = c.y() * rs;
		z = c.z() * rs;
		w = s * 0.5f;
	}

	VALIDATE(*this);
}

T_MATH_INLINE Quaternion Quaternion::identity()
{
	return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

T_MATH_INLINE Quaternion Quaternion::normalized() const
{
	VALIDATE(*this);
	float i = 1.0f / sqrtf(x * x + y * y + z * z + w * w);
	T_ASSERT (i > FUZZY_EPSILON);
	return Quaternion(x * i, y * i, z * i, w * i);
}

T_MATH_INLINE Quaternion Quaternion::inverse() const
{
	VALIDATE(*this);
	return Quaternion(-x, -y, -z, w);
}

T_MATH_INLINE Vector4 Quaternion::toAxisAngle() const
{
	VALIDATE(*this);

	float scale = sqrtf(x * x + y * y + z * z);
	if (scale <= 0.0f)
		return Vector4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 2.0f * acosf(w);
	return Vector4(
		x / scale * angle,
		y / scale * angle,
		z / scale * angle,
		0.0f
	);
}

T_MATH_INLINE Matrix44 Quaternion::toMatrix44() const
{
	VALIDATE(*this);
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

T_MATH_INLINE void Quaternion::toEulerAngles(float& outHead, float& outPitch, float& outBank) const
{
	VALIDATE(*this);
	outHead  = atan2f(2.0f * y * w - 2.0f * x * z, 1.0f - 2.0f * y * y - 2.0f * z *z);
	outPitch = atan2f(2.0f * x * w - 2.0f * y * z, 1.0f - 2.0f * x * x - 2 * z * z);
	outBank  = asinf (2.0f * x * y + 2.0f * z * w);
}

T_MATH_INLINE Quaternion& Quaternion::operator *= (const Quaternion& r)
{
	VALIDATE(*this);
	VALIDATE(r);
	*this = *this * r;
	return *this;
}

T_MATH_INLINE bool Quaternion::operator == (const Quaternion& v) const
{
	VALIDATE(*this);
	VALIDATE(v);
	return v.x == x && v.y == y && v.z == z && v.w == w;
}

T_MATH_INLINE bool Quaternion::operator != (const Quaternion& v) const
{
	VALIDATE(*this);
	VALIDATE(v);
	return v.x != x || v.y != y || v.z != z || v.w != w;
}

T_MATH_INLINE Quaternion operator + (const Quaternion& l, const Quaternion& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Quaternion(
		l.x + r.x,
		l.y + r.y,
		l.z + r.z,
		l.w + r.w
	);
}

T_MATH_INLINE Quaternion operator * (const Quaternion& l, float r)
{
	VALIDATE(l);
	return Quaternion(
		l.x * r,
		l.y * r,
		l.z * r,
		l.w * r
	);
}

T_MATH_INLINE Quaternion operator * (const Quaternion& l, const Quaternion& r)
{
	VALIDATE(l);
	VALIDATE(r);
	return Quaternion(
		l.w * r.x + l.x * r.w + l.y * r.z - l.z * r.y,
		l.w * r.y + l.y * r.w + l.z * r.x - l.x * r.z,
		l.w * r.z + l.z * r.w + l.x * r.y - l.y * r.x,
		l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z
	);
}

T_MATH_INLINE Vector4 operator * (const Quaternion& q, const Vector4& v)
{
	VALIDATE(q);
	Quaternion qv(v.x(), v.y(), v.z(), 0.0f);
	Quaternion qvp = q * qv * q.inverse();
	return Vector4(qvp.x, qvp.y, qvp.z, v.w());
}

T_MATH_INLINE Quaternion lerp(const Quaternion& a, const Quaternion& b, float c)
{
	VALIDATE(a);
	VALIDATE(b);
	return slerp(a, b, c);
}

T_MATH_INLINE Quaternion slerp(const Quaternion& a, const Quaternion& b, float c)
{
	VALIDATE(a);
	VALIDATE(b);

	float scale1;
	float scale2;
	Quaternion A = a;
	Quaternion B = b;

	float cosTheta = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
	if (cosTheta < 0.0f)
	{
		A.x = -A.x;
		A.y = -A.y;
		A.z = -A.z;
		A.w = -A.w;
		cosTheta = -cosTheta;
	}

	if ((cosTheta + 1.0f) > 0.05f)
	{
		if ((1.0f - cosTheta) < 0.05f)
		{
			scale1 = 1.0f - c;
			scale2 = c;
		}
		else
		{
			float theta = acosf(cosTheta);
			float sinTheta = sinf(theta);
			scale1 = sinf(theta * (1.0f - c)) / sinTheta;
			scale2 = sinf(theta * c) / sinTheta;
		}
	}
	else
	{
		B.x = -A.y;
		B.y =  A.x;
		B.z = -A.w;
		B.w =  A.z;
		scale1 = sinf(PI * (0.5f - c));
		scale2 = sinf(PI * c);
	}

	return Quaternion(
		scale1 * A.x + scale2 * B.x,
		scale1 * A.y + scale2 * B.y,
		scale1 * A.z + scale2 * B.z,
		scale1 * A.w + scale2 * B.w
	);
}

#if defined(VALIDATE)
#undef VALIDATE
#endif

}

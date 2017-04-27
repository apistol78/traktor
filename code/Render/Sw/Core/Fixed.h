/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_FixedPoint_H
#define traktor_render_FixedPoint_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

typedef int fp32_t;

inline fp32_t toFixed(int v)
{
	return v << 16;
}

inline fp32_t toFixed(float v)
{
	return fp32_t(v * 65536.0f);
}

inline int toInt(fp32_t v)
{
	return v >> 16;
}

inline float toFloat(fp32_t v)
{
	return float(v) / 65536.0f;
}

inline fp32_t fixedMul(fp32_t a, fp32_t b)
{
	return fp32_t((int64_t(a) * int64_t(b)) >> 16);
}

inline fp32_t fixedDiv(fp32_t a, fp32_t b)
{
	return b ? fp32_t((int64_t(a) << 16) / b) : 0x7fffffff;
}

inline fp32_t fixedLog(fp32_t a)
{
	return 0;
}

inline fp32_t fixedLog2(fp32_t a)
{
	return 0;
}

inline fp32_t fixedLog10(fp32_t a)
{
	return 0;
}

inline fp32_t fixedPow(fp32_t a, fp32_t b)
{
	return toFixed(float(std::pow(toFloat(a), toFloat(b))));
}

inline fp32_t fixedSqrt(fp32_t a)
{
	fp32_t s = (a + 65536) >> 1;
	for (uint32_t i = 0; i < 6; ++i)
		s = (s + fixedDiv(a, s)) >> 1;
	return s;
}

inline fp32_t fixedArcCos(fp32_t a)
{
	return toFixed(float(std::acos(toFloat(a))));
}

inline fp32_t fixedArcTan(fp32_t a)
{
	return toFixed(float(std::atan(toFloat(a))));
}

inline fp32_t fixedCos(fp32_t a)
{
	return toFixed(float(std::cos(toFloat(a))));
}

inline fp32_t fixedSin(fp32_t a)
{
	return toFixed(float(std::sin(toFloat(a))));
}

inline fp32_t fixedTan(fp32_t a)
{
	return toFixed(float(std::tan(toFloat(a))));
}

const fp32_t c_zero = 0x00000000;
const fp32_t c_one = 0x00010000;

class Fix4
{
public:
	union
	{
		fp32_t e[4];
		struct { fp32_t x, y, z, w; };
	};

	inline Fix4();

	inline Fix4(fp32_t x, fp32_t y, fp32_t z, fp32_t w);

	inline void set(fp32_t x, fp32_t y, fp32_t z, fp32_t w);

	inline Fix4& operator = (const Fix4& r);

	inline Fix4 operator - () const;

	friend inline Fix4 operator + (const Fix4& l, const Fix4& r);

	friend inline Fix4 operator - (const Fix4& l, const Fix4& r);

	friend inline Fix4 operator * (const Fix4& l, fp32_t r);

	friend inline Fix4 operator * (const Fix4& l, const Fix4& r);

	friend inline Fix4 operator / (const Fix4& l, const Fix4& r);

	friend inline Fix4 operator / (const Fix4& l, fp32_t r);
};

Fix4::Fix4()
{
}

Fix4::Fix4(fp32_t x_, fp32_t y_, fp32_t z_, fp32_t w_)
:	x(x_)
,	y(y_)
,	z(z_)
,	w(w_)
{
}

inline void Fix4::set(fp32_t x_, fp32_t y_, fp32_t z_, fp32_t w_)
{
	x = x_;
	y = y_;
	z = z_;
	w = w_;
}

Fix4& Fix4::operator = (const Fix4& r)
{
	x = r.x;
	y = r.y;
	z = r.z;
	w = r.w;
	return *this;
}

inline Fix4 Fix4::operator - () const
{
	return Fix4(-x, -y, -z, -w);
}

inline Fix4 operator + (const Fix4& l, const Fix4& r)
{
	return Fix4(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
}

inline Fix4 operator - (const Fix4& l, const Fix4& r)
{
	return Fix4(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
}

inline Fix4 operator * (const Fix4& l, fp32_t r)
{
	return Fix4(
		fixedMul(l.x, r),
		fixedMul(l.y, r),
		fixedMul(l.z, r),
		fixedMul(l.w, r)
	);
}

inline Fix4 operator * (const Fix4& l, const Fix4& r)
{
	return Fix4(
		fixedMul(l.x, r.x),
		fixedMul(l.y, r.y),
		fixedMul(l.z, r.z),
		fixedMul(l.w, r.w)
	);
}

inline Fix4 operator / (const Fix4& l, fp32_t r)
{
	return Fix4(
		fixedDiv(l.x, r),
		fixedDiv(l.y, r),
		fixedDiv(l.z, r),
		fixedDiv(l.w, r)
	);
}

inline Fix4 operator / (const Fix4& l, const Fix4& r)
{
	return Fix4(
		fixedDiv(l.x, r.x),
		fixedDiv(l.y, r.y),
		fixedDiv(l.z, r.z),
		fixedDiv(l.w, r.w)
	);
}

inline fp32_t dot3(const Fix4& l, const Fix4& r)
{
	return fixedMul(l.x, r.x) + fixedMul(l.y, r.y) + fixedMul(l.z, r.z);
}

inline fp32_t dot4(const Fix4& l, const Fix4& r)
{
	return fixedMul(l.x, r.x) + fixedMul(l.y, r.y) + fixedMul(l.z, r.z) + fixedMul(l.w, r.w);
}

inline Fix4 cross(const Fix4& l, const Fix4& r)
{
	return Fix4(
		fixedMul(l.y, r.z) - fixedMul(r.y, l.z),
		fixedMul(l.z, r.x) - fixedMul(r.z, l.x),
		fixedMul(l.x, r.y) - fixedMul(r.x, l.y),
		c_zero
	);
}

inline Fix4 toFixed(const Vector4& v)
{
	return Fix4(
		toFixed(v.x()),
		toFixed(v.y()),
		toFixed(v.z()),
		toFixed(v.w())
	);
}

inline Vector4 toFloat(const Fix4& v)
{
	return Vector4(
		toFloat(v.x),
		toFloat(v.y),
		toFloat(v.z),
		toFloat(v.w)
	);
}

//@}

	}
}

#endif	// traktor_render_FixedPoint_H

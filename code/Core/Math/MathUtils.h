/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MathUtils_H
#define traktor_MathUtils_H

#include <cmath>
#if defined(_XBOX)
#	include <ppcintrinsics.h>
#endif
#if defined(_MSC_VER)
#	include "Core/Math/Scalar.h"
#endif

#if defined(min)
#	undef min
#endif
#if defined(max)
#	undef max
#endif

namespace traktor
{

/*! \ingroup Core */
//@{

template < typename Type >
inline Type min(Type a, Type b)
{
	return a < b ? a : b;
}

template < typename Type >
inline Type min(Type a, Type b, Type c)
{
	return min(a, min(b, c));
}

template < typename Type >
inline Type max(Type a, Type b)
{
	return a > b ? a : b;
}

template < typename Type >
inline Type max(Type a, Type b, Type c)
{
	return max(a, max(b, c));
}

template < typename Type >
inline Type abs(Type a)
{
	return a < 0 ? -a : a;
}

template < typename Type >
inline Type select(Type a, Type b, Type c)
{
	return a >= 0 ? b : c;
}

template < typename Type >
inline Type sign(Type a)
{
	return a >= 0 ? Type(1) : Type(-1);
}

#if defined(_MSC_VER)

inline Scalar min(const Scalar& a, const Scalar& b)
{
	return a < b ? a : b;
}

inline Scalar max(const Scalar& a, const Scalar& b)
{
	return a > b ? a : b;
}

inline Scalar select(const Scalar& a, const Scalar& b, const Scalar& c)
{
	return a >= 0 ? b : c;
}

inline Scalar sign(const Scalar& a)
{
	return a >= 0 ? Scalar(1.0f) : Scalar(-1.0f);
}

#endif

#if defined(_XBOX)

template < >
inline float min(float a, float b)
{
	return float(__fsel(a - b, b, a));
}

template < >
inline float max(float a, float b)
{
	return float(__fsel(a - b, a, b));
}

template < >
inline float abs(float a)
{
	return float(__fsel(a, a, -a));
}

template < >
inline float select(float a, float b, float c)
{
	return float(__fsel(a, b, c));
}

template < >
inline float sign(float a)
{
	return float(__fsel(a, 1.0f, -1.0f));
}

#endif

template < typename Type >
inline Type clamp(Type value, Type minLimit, Type maxLimit)
{
	return max(min(value, maxLimit), minLimit);
}

#if defined(_MSC_VER) || defined(_PS3)

inline float sinf(float rad)
{
	return std::sinf(rad);
}

inline float cosf(float rad)
{
	return std::cosf(rad);
}

inline float asinf(float phi)
{
	return std::asinf(phi);
}

inline float acosf(float phi)
{
	return std::acosf(phi);
}

inline float atan2f(float x, float y)
{
	return std::atan2f(x, y);
}

inline float sqrtf(float x)
{
	return std::sqrtf(x);
}

inline float powf(float x, float e)
{
	return std::powf(x, e);
}

#else

inline float sinf(float rad)
{
	return (float)sin(rad);
}

inline float cosf(float rad)
{
	return (float)cos(rad);
}

inline float asinf(float phi)
{
	return (float)asin(phi);
}

inline float acosf(float phi)
{
	return (float)acos(phi);
}

inline float atan2f(float x, float y)
{
	return (float)atan2(x, y);
}

inline float sqrtf(float x)
{
	return (float)sqrt(x);
}

inline float powf(float x, float e)
{
	return (float)::powf(x, e);
}

#endif

//@}

}

#endif	// traktor_MathUtils_H

#ifndef traktor_Color4f_H
#define traktor_Color4f_H

#include "Core/Config.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief High range color.
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Color4f
{
public:
	T_MATH_INLINE Color4f();

	T_MATH_INLINE Color4f(const Color4f& src);

	T_MATH_INLINE Color4f(float red, float green, float blue, float alpha = 0.0f);

	T_MATH_INLINE explicit Color4f(const float* data);

	T_MATH_INLINE explicit Color4f(const Vector4& data);

	T_MATH_INLINE Scalar get(int channel) const;

	T_MATH_INLINE Scalar getRed() const;

	T_MATH_INLINE Scalar getGreen() const;

	T_MATH_INLINE Scalar getBlue() const;

	T_MATH_INLINE Scalar getAlpha() const;

	T_MATH_INLINE void set(int channel, const Scalar& value);
	
	T_MATH_INLINE void set(float red, float green, float blue, float alpha = 0.0f);

	T_MATH_INLINE void setRed(const Scalar& red);

	T_MATH_INLINE void setGreen(const Scalar& green);

	T_MATH_INLINE void setBlue(const Scalar& blue);

	T_MATH_INLINE void setAlpha(const Scalar& alpha);

	T_MATH_INLINE Color4f saturated() const;

	T_MATH_INLINE Color4f rgb0() const;

	T_MATH_INLINE Color4f rgb1() const;

	T_MATH_INLINE Color4ub toColor4ub() const;

	static T_MATH_INLINE Color4f loadAligned(const float* in);

	static T_MATH_INLINE Color4f loadUnaligned(const float* in);

	T_MATH_INLINE void storeAligned(float* out) const;

	T_MATH_INLINE void storeUnaligned(float* out) const;

	T_MATH_INLINE Color4f& operator = (const Color4f& src);

	T_MATH_INLINE Color4f operator + (const Color4f& r) const;

	T_MATH_INLINE Color4f operator - (const Color4f& r) const;

	T_MATH_INLINE Color4f operator * (const Color4f& r) const;

	T_MATH_INLINE Color4f operator * (const Scalar& r) const;

	T_MATH_INLINE Color4f operator / (const Scalar& r) const;

	T_MATH_INLINE Color4f& operator += (const Color4f& r);

	T_MATH_INLINE Color4f& operator -= (const Color4f& r);

	T_MATH_INLINE Color4f& operator *= (const Scalar& r);

	T_MATH_INLINE Color4f& operator /= (const Scalar& r);

	T_MATH_INLINE bool operator == (const Color4f& r) const;

	T_MATH_INLINE bool operator != (const Color4f& r) const;

	T_MATH_INLINE operator const Vector4& () const;

private:
	Vector4 m_data;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Color4f.inl"
#endif

#endif	// traktor_Color4f_H

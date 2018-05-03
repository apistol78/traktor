/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Color_H
#define traktor_Color_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Color; 4 unsigned bytes representation.
 * \ingroup Core
 */
class T_DLLCLASS Color4ub
{
public:
#if defined (_MSC_VER)
#	pragma warning( disable:4201 )
#endif
	union
	{
		uint8_t e[4];
		struct { uint8_t r, g, b, a; };
	};
#if defined (_MSC_VER)
#	pragma warning( default:4201 )
#endif
	
	T_MATH_INLINE Color4ub();

	T_MATH_INLINE Color4ub(const Color4ub& v);

	explicit T_MATH_INLINE Color4ub(uint8_t r_, uint8_t g_, uint8_t b_);

	explicit T_MATH_INLINE Color4ub(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_);

	explicit T_MATH_INLINE Color4ub(uint32_t argb);

	T_MATH_INLINE void set(uint8_t r_, uint8_t g_, uint8_t b_);

	T_MATH_INLINE void set(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_);

	T_MATH_INLINE uint32_t getARGB() const;

	T_MATH_INLINE uint32_t getABGR() const;

	T_MATH_INLINE uint32_t getRGBA() const;

	T_MATH_INLINE uint32_t getBGRA() const;

	T_MATH_INLINE void getRGBA32F(float outRGBA[4]) const;

	T_MATH_INLINE uint16_t getRGB565() const;

	T_MATH_INLINE Color4ub& operator = (const Color4ub& v);

	T_MATH_INLINE Color4ub operator + (const Color4ub& color) const;

	T_MATH_INLINE Color4ub operator * (const Color4ub& color) const;

	T_MATH_INLINE bool operator == (const Color4ub& v) const;

	T_MATH_INLINE bool operator != (const Color4ub& v) const;

	T_MATH_INLINE operator uint32_t () const;
};

T_MATH_INLINE T_DLLCLASS Color4ub lerp(const Color4ub& a, const Color4ub& b, float c);

}

#if defined(T_MATH_USE_INLINE)
#include "Core/Math/Std/Color4ub.inl"
#endif

#endif	// traktor_Color_H

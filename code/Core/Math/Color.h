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

/*! \brief Color
 * \ingroup Core
 */
class T_DLLCLASS Color
{
public:
	union
	{
		uint8_t e[4];
		struct { uint8_t r, g, b, a; };
	};
	
	T_MATH_INLINE Color();

	T_MATH_INLINE Color(const Color& v);

	explicit T_MATH_INLINE Color(uint8_t r, uint8_t g, uint8_t b);

	explicit T_MATH_INLINE Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	explicit T_MATH_INLINE Color(uint32_t argb);

	T_MATH_INLINE void set(uint8_t r, uint8_t g, uint8_t b);

	T_MATH_INLINE void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	T_MATH_INLINE uint32_t getARGB() const;

	T_MATH_INLINE uint32_t getABGR() const;

	T_MATH_INLINE uint32_t getRGBA() const;

	T_MATH_INLINE uint32_t getBGRA() const;

	T_MATH_INLINE Color& operator = (const Color& v);

	T_MATH_INLINE Color operator + (const Color& color) const;

	T_MATH_INLINE Color operator * (const Color& color) const;

	T_MATH_INLINE bool operator == (const Color& v) const;

	T_MATH_INLINE bool operator != (const Color& v) const;

	T_MATH_INLINE operator uint32_t () const;
};

T_MATH_INLINE T_DLLCLASS Color lerp(const Color& a, const Color& b, float c);

}

#if defined(T_MATH_USE_INLINE)
#include "Core/Math/Std/Color.inl"
#endif

#endif	// traktor_Color_H

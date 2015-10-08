#ifndef traktor_Vector2i_H
#define traktor_Vector2i_H

#include <cmath>
#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief 2d vector using 32-bit integers.
 * \ingroup Core
 */
class T_DLLCLASS Vector2i
{
public:
	union
	{
		int32_t e[2];
		struct { int32_t x, y; };
	};

	T_MATH_INLINE Vector2i();

	T_MATH_INLINE Vector2i(const Vector2i& v);

	explicit T_MATH_INLINE Vector2i(int32_t x_, int32_t y_);

	T_MATH_INLINE Vector2 toVector2() const;

	static T_MATH_INLINE Vector2i fromVector2(const Vector2& v);

	T_MATH_INLINE bool operator == (const Vector2i& r) const;

	T_MATH_INLINE bool operator != (const Vector2i& r) const;

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2i operator + (const Vector2i& l, const Vector2i& r);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2i operator - (const Vector2i& l, const Vector2i& r);
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Vector2i.inl"
#endif

#endif	// traktor_flash_Vector2i_H

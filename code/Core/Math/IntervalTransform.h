#ifndef traktor_IntervalTransform_H
#define traktor_IntervalTransform_H

#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_MATH_ALIGN16 T_DLLCLASS IntervalTransform
{
public:
	T_MATH_INLINE IntervalTransform();

	T_MATH_INLINE IntervalTransform(const Transform& transform);

	T_MATH_INLINE IntervalTransform(const Transform& transform0, const Transform& transform1);

	T_MATH_INLINE void set(const Transform& transform);

	T_MATH_INLINE Transform get(float interval) const;

	T_MATH_INLINE void step();

	T_MATH_INLINE const Transform& get0() const;

	T_MATH_INLINE const Transform& get() const;

private:
	Transform m_transform[2];
	bool m_stepped;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/IntervalTransform.inl"
#endif

#endif	// traktor_IntervalTransform_H

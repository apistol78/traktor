#pragma once

#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
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

class T_DLLCLASS Polar
{
public:
	float phi;
	float theta;

	Polar() = default;

	T_MATH_INLINE explicit Polar(float phi_, float theta_);

	static T_MATH_INLINE Polar fromUnitCartesian(const Vector4& unit);

	T_MATH_INLINE Vector4 toUnitCartesian() const;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Polar.inl"
#endif

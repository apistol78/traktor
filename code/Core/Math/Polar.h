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

	T_MATH_INLINE explicit Polar(float phi_, float theta_)
	:	phi(phi_)
	,	theta(theta_)
	{
	}

	static T_MATH_INLINE Polar fromUnitCartesian(const Vector4& unit)
	{
		float phi = acosf(unit.y());
		float theta = atan2f(unit.z(), unit.x());
		return Polar(phi, theta >= 0.0f ? theta : theta + TWO_PI);
	}

	T_MATH_INLINE Vector4 toUnitCartesian() const
	{
		return Vector4(
			sinf(phi) * cosf(theta),
			cosf(phi),
			sinf(phi) * sinf(theta)
		);
	}
};

}

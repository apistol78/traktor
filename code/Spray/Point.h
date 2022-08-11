#pragma once

#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace spray
	{

/*! Particle point.
 * \ingroup Spray
 */
struct T_MATH_ALIGN16 Point
{
	Vector4 position;
	Vector4 velocity;
	Vector4 color;		// 48
	float orientation;
	float angularVelocity;
	float inverseMass;
	float age;
	float maxAge;
	float size;
	float random;		// 28
};

/*! Array of particles.
 * \ingroup Spray
 */
typedef AlignedVector< Point > pointVector_t;

	}
}


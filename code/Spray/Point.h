#ifndef traktor_spray_Point_H
#define traktor_spray_Point_H

#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Particle point.
 * \ingroup Spray
 */
struct Point
{
	Vector4 position;
	Vector4 velocity;
	float orientation;
	float angularVelocity;
	Vector4 color;
	float inverseMass;
	float age;
	float maxAge;
	float size;
	float random;
};

/*! \brief Array of particles.
 * \ingroup Spray
 */
typedef AlignedVector< Point > PointVector;

	}
}

#endif	// traktor_spray_Point_H

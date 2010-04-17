#ifndef traktor_spray_Point_H
#define traktor_spray_Point_H

#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace spray
	{

#pragma pack(1)

/*! \brief Particle point.
 * \ingroup Spray
 */
struct Point
{
	Vector4 position;
	Vector4 velocity;
	Vector4 color;
	float orientation;
	float angularVelocity;
	float inverseMass;
	float age;
	float maxAge;
	float size;
	float random;

	// Pad to ensure structure is a multiple of 16 bytes.
	uint8_t pad[4];
};

#pragma pack()

/*! \brief Array of particles.
 * \ingroup Spray
 */
typedef AlignedVector< Point > PointVector;

	}
}

#endif	// traktor_spray_Point_H

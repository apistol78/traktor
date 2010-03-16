#ifndef traktor_spray_JobModifierUpdate_H
#define traktor_spray_JobModifierUpdate_H

#include <cell/spurs/job_descriptor.h>
#include "Core/Math/Transform.h"

namespace traktor
{
	namespace spray
	{
		
struct JobModifierUpdate
{
	CellSpursJobHeader header;
	Transform transform;
	float deltaTime;
	uintptr_t pointsEA;
	uint32_t pointsCount;

	union
	{
		struct
		{
			float linearDrag;
			float angularDrag;
		}
		drag;

		struct 
		{
			float gravity[4];
			uint8_t world;
		}
		gravity;

		struct 
		{
			float timeScale;
		}
		integrate;

		struct 
		{
			float plane[4];
			float restitution;
		}
		planeCollision;

		struct 
		{
			float adjustRate;
		}
		size;

		struct 
		{
			float axis[4];
			float tangentForce;
			float normalConstantForce;
			float normalDistance;
			float normalDistanceForce;
		}
		vortex;
	};
};

	}
}

#endif	// traktor_spray_JobModifierUpdate_H

#ifndef traktor_spray_Types_H
#define traktor_spray_Types_H

#include "Core/Math/RandomGeometry.h"

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;

	}

	namespace world
	{

class IEntityEventManager;

	}

	namespace spray
	{

/*! \brief Effect context.
 * \ingroup Spray
 */
struct Context
{
	float deltaTime;
	RandomGeometry random;
	world::IEntityEventManager* eventManager;
	sound::ISoundPlayer* soundPlayer;
};

	}
}

#endif	// traktor_spray_Types_H

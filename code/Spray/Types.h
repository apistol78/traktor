#ifndef traktor_spray_Types_H
#define traktor_spray_Types_H

#include "Core/Math/RandomGeometry.h"

namespace traktor
{
	namespace sound
	{

class SoundSystem;
class SurroundEnvironment;

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
	sound::SoundSystem* soundSystem;
	sound::SurroundEnvironment* surroundEnvironment;
};

	}
}

#endif	// traktor_spray_Types_H

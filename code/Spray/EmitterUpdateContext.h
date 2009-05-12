#ifndef traktor_spray_EmitterRenderContext_H
#define traktor_spray_EmitterRenderContext_H

#include "Core/Math/RandomGeometry.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Emitter update environment.
 * \ingroup Spray
 */
struct EmitterUpdateContext
{
	float deltaTime;
	RandomGeometry random;

	EmitterUpdateContext(float deltaTime_ = 0.0f, uint32_t randomSeed = 5489UL)
	:	deltaTime(deltaTime_)
	,	random(randomSeed)
	{
	}
};

	}
}

#endif	// traktor_spray_EmitterRenderContext_H

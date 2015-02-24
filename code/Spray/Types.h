#ifndef traktor_spray_Types_H
#define traktor_spray_Types_H

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Transform.h"

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;

	}

	namespace world
	{

class Entity;
class IEntityEventManager;

	}

	namespace spray
	{

/*! \brief Virtual source callback.
 * \ingroup Spray
 */
struct VirtualSourceCallback
{
	virtual ~VirtualSourceCallback() {}

	virtual void virtualSourceEmit(
		const class VirtualSource* source,
		struct Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		class EmitterInstance& emitterInstance
	) = 0;
};

/*! \brief Effect context.
 * \ingroup Spray
 */
struct Context
{
	float deltaTime;
	RandomGeometry random;
	world::Entity* owner;
	world::IEntityEventManager* eventManager;
	sound::ISoundPlayer* soundPlayer;
	VirtualSourceCallback* virtualSourceCallback;

	Context()
	:	deltaTime(0.0f)
	,	owner(0)
	,	eventManager(0)
	,	soundPlayer(0)
	,	virtualSourceCallback(0)
	{
	}
};

	}
}

#endif	// traktor_spray_Types_H

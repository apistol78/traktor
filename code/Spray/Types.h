#pragma once

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
class EntityEventManager;

	}

	namespace spray
	{

/*! Virtual source callback.
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

/*! Effect context.
 * \ingroup Spray
 */
struct Context
{
	float deltaTime;
	RandomGeometry random;
	world::Entity* owner;
	world::EntityEventManager* eventManager;
	sound::ISoundPlayer* soundPlayer;
	VirtualSourceCallback* virtualSourceCallback;

	Context()
	:	deltaTime(0.0f)
	,	owner(nullptr)
	,	eventManager(nullptr)
	,	soundPlayer(nullptr)
	,	virtualSourceCallback(nullptr)
	{
	}
};

	}
}


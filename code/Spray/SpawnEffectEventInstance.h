#pragma once

#include "Core/Math/Transform.h"
#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class ComponentEntity;
class Entity;

	}

	namespace spray
	{

class EffectComponent;
class SpawnEffectEvent;

/*! Spawn effect event instance.
 * \ingroup Spray
 */
class SpawnEffectEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	SpawnEffectEventInstance(
		const SpawnEffectEvent* spawnEffect,
		world::Entity* sender,
		const Transform& Toffset,
		EffectComponent* effectComponent
	);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void attach(world::IWorldRenderer* worldRenderer) override final;

	virtual void cancel(world::CancelType when) override final;

private:
	const SpawnEffectEvent* m_spawnEffect;
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	Ref< EffectComponent > m_effectComponent;
	Ref< world::ComponentEntity > m_effectEntity;
};

	}
}


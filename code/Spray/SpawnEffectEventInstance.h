#pragma once

#include "Core/Math/Transform.h"
#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace spray
	{

class EffectEntity;
class SpawnEffectEvent;

class SpawnEffectEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	SpawnEffectEventInstance(const SpawnEffectEvent* spawnEffect, world::Entity* sender, const Transform& Toffset, EffectEntity* effectEntity);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void build(world::IWorldRenderer* worldRenderer) override final;

	virtual void cancel(world::CancelType when) override final;

private:
	const SpawnEffectEvent* m_spawnEffect;
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	Ref< EffectEntity > m_effectEntity;
};

	}
}


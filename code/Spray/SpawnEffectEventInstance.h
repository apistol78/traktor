/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_SpawnEffectEventInstance_H
#define traktor_spray_SpawnEffectEventInstance_H

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

	virtual bool update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void build(world::IWorldRenderer* worldRenderer) T_OVERRIDE T_FINAL;

	virtual void cancel(world::CancelType when) T_OVERRIDE T_FINAL;

private:
	const SpawnEffectEvent* m_spawnEffect;
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	Ref< EffectEntity > m_effectEntity;
};

	}
}

#endif	// traktor_spray_SpawnEffectEventInstance_H

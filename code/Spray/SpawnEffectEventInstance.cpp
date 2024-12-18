/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Spray/EffectComponent.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventInstance.h"
#include "World/IWorldRenderer.h"
#include "World/Entity.h"
#include "World/World.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SpawnEffectEventInstance", SpawnEffectEventInstance, world::IEntityEventInstance)

SpawnEffectEventInstance::SpawnEffectEventInstance(
	const SpawnEffectEvent* spawnEffect,
	world::World* world,
	world::Entity* sender,
	const Transform& Toffset,
	EffectComponent* effectComponent
)
:	m_spawnEffect(spawnEffect)
,	m_world(world)
,	m_sender(sender)
,	m_Toffset(Toffset)
{
	Transform T;
	if (m_sender)
		T = m_sender->getTransform() * m_Toffset;
	else
		T = m_Toffset;

	m_effectEntity = new world::Entity();
	m_effectEntity->setComponent(effectComponent);

	if (m_spawnEffect->m_useRotation)
		m_effectEntity->setTransform(T);
	else
		m_effectEntity->setTransform(Transform(T.translation()));

	m_world->addEntity(m_effectEntity);
}

SpawnEffectEventInstance::~SpawnEffectEventInstance()
{
	cancel(world::Cancel::Immediate);
}

bool SpawnEffectEventInstance::update(world::World* world, const world::UpdateParams& update)
{
	if (!m_effectEntity)
		return false;

	if (m_spawnEffect->m_follow)
	{
		Transform T;
		if (m_sender)
			T = m_sender->getTransform() * m_Toffset;
		else
			T = m_Toffset;

		if (m_spawnEffect->m_useRotation)
			m_effectEntity->setTransform(T);
		else
			m_effectEntity->setTransform(Transform(T.translation()));
	}

	if (m_effectEntity->getComponent< EffectComponent >()->isFinished())
	{
		m_world->removeEntity(m_effectEntity);
		safeDestroy(m_effectEntity);
		return false;	
	}

	return true;
}

void SpawnEffectEventInstance::cancel(world::Cancel when)
{
	if (!m_effectEntity)
		return;

	if (when == world::Cancel::Immediate)
	{
		m_world->removeEntity(m_effectEntity);
		safeDestroy(m_effectEntity);
	}
	else
	{
		m_effectEntity->getComponent< EffectComponent >()->setLoopEnable(false);
	}
}

}

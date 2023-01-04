/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SpawnEffectEventInstance", SpawnEffectEventInstance, world::IEntityEventInstance)

SpawnEffectEventInstance::SpawnEffectEventInstance(
	const SpawnEffectEvent* spawnEffect,
	world::Entity* sender,
	const Transform& Toffset,
	EffectComponent* effectComponent
)
:	m_spawnEffect(spawnEffect)
,	m_sender(sender)
,	m_Toffset(Toffset)
,	m_effectComponent(effectComponent)
{
	Transform T;
	if (m_sender)
		T = m_sender->getTransform() * m_Toffset;
	else
		T = m_Toffset;

	m_effectEntity = new world::Entity();
	m_effectEntity->setComponent(m_effectComponent);

	if (m_spawnEffect->m_useRotation)
		m_effectEntity->setTransform(T);
	else
		m_effectEntity->setTransform(Transform(T.translation()));
}

bool SpawnEffectEventInstance::update(const world::UpdateParams& update)
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

	m_effectEntity->update(update);

	return !m_effectComponent->isFinished();
}

void SpawnEffectEventInstance::gather(const std::function< void(world::Entity*) >& fn) const
{
	fn(m_effectEntity);
}

void SpawnEffectEventInstance::cancel(world::Cancel when)
{
	if (when == world::Cancel::Immediate)
		safeDestroy(m_effectEntity);
	else
	{
		if (m_effectComponent)
			m_effectComponent->setLoopEnable(false);
	}
}

}

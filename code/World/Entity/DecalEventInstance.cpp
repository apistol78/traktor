/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "World/IWorldRenderer.h"
#include "World/Entity.h"
#include "World/World.h"
#include "World/Entity/DecalComponent.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventInstance.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEventInstance", DecalEventInstance, IEntityEventInstance)

DecalEventInstance::DecalEventInstance(const DecalEvent* event, World* world, const Transform& Toffset)
:	m_world(world)
{
	m_entity = new Entity();
	m_entity->setComponent(new DecalComponent(
		event->getSize(),
		event->getThickness(),
		event->getAlpha(),
		event->getCullDistance(),
		event->getShader()
	));
	m_entity->setTransform(Toffset);
	m_world->addEntity(m_entity);
}

bool DecalEventInstance::update(world::World* world, const UpdateParams& update)
{
	if (m_entity)
	{
		m_entity->update(update);
		if (m_entity->getComponent< DecalComponent >()->getAlpha() > FUZZY_EPSILON)
			return true;
	}
	return false;
}

void DecalEventInstance::cancel(Cancel when)
{
	if (m_entity)
	{
		m_world->removeEntity(m_entity);
		safeDestroy(m_entity);
	}
}

}

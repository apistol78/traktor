/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"
#include "World/IEntityFactory.h"
#include "World/World.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, IEntityBuilder)

EntityBuilder::EntityBuilder(const IEntityFactory* entityFactory, World* world)
:	m_entityFactory(entityFactory)
,	m_world(world)
{
}

Ref< Entity > EntityBuilder::create(const EntityData* entityData) const
{
	Ref< Entity > entity = m_entityFactory->createEntity(this, *entityData);
	if (!entity)
		return nullptr;

	// Do not add if data is external, has already been added.
	if (!is_a< ExternalEntityData >(entityData))
		m_world->addEntity(entity);

	return entity;
}

Ref< IEntityEvent > EntityBuilder::create(const IEntityEventData* entityEventData) const
{
	return m_entityFactory->createEntityEvent(this, *entityEventData);
}

Ref< IEntityComponent > EntityBuilder::create(const IEntityComponentData* entityComponentData) const
{
	return m_entityFactory->createEntityComponent(this, *entityComponentData);
}

}

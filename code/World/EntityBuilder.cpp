/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, IEntityBuilder)

EntityBuilder::EntityBuilder(const IEntityFactory* entityFactory)
	: m_entityFactory(entityFactory)
{
}

Ref< Entity > EntityBuilder::create(const EntityData* entityData) const
{
	return m_entityFactory->createEntity(this, *entityData);
}

Ref< IEntityEvent > EntityBuilder::create(const IEntityEventData* entityEventData) const
{
	return m_entityFactory->createEntityEvent(this, *entityEventData);
}

Ref< IEntityComponent > EntityBuilder::create(const IEntityComponentData* entityComponentData) const
{
	return m_entityFactory->createEntityComponent(this, *entityComponentData);
}

Ref< IWorldComponent > EntityBuilder::create(const IWorldComponentData* worldComponentData) const
{
	return m_entityFactory->createWorldComponent(this, *worldComponentData);
}

}

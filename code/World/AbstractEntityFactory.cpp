/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/AbstractEntityFactory.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.AbstractEntityFactory", AbstractEntityFactory, IEntityFactory)

const TypeInfoSet AbstractEntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AbstractEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AbstractEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AbstractEntityFactory::getWorldComponentTypes() const
{
	return TypeInfoSet();
}

Ref< Entity > AbstractEntityFactory::createEntity(const IEntityBuilder* builder, const EntityData& entityData) const
{
	return nullptr;
}

Ref< IEntityEvent > AbstractEntityFactory::createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< IEntityComponent > AbstractEntityFactory::createEntityComponent(const IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

Ref< IWorldComponent > AbstractEntityFactory::createWorldComponent(const IEntityBuilder* builder, const IWorldComponentData& worldComponentData) const
{
	return nullptr;
}

}

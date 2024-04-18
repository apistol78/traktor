/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ObjectStore;

}

namespace traktor::world
{

class IEntityBuilder;
class IEntityComponent;
class IEntityComponentData;
class IEntityEvent;
class IEntityEventData;
class IWorldComponent;
class IWorldComponentData;
class Entity;
class EntityData;

/*! Entity factory interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool initialize(const ObjectStore& objectStore) = 0;

	virtual const TypeInfoSet getEntityTypes() const = 0;

	virtual const TypeInfoSet getEntityEventTypes() const = 0;

	virtual const TypeInfoSet getEntityComponentTypes() const = 0;

	virtual const TypeInfoSet getWorldComponentTypes() const = 0;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const = 0;

	virtual Ref< IEntityEvent > createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const = 0;

	virtual Ref< IEntityComponent > createEntityComponent(const IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const = 0;

	virtual Ref< IWorldComponent > createWorldComponent(const IEntityBuilder* builder, const IWorldComponentData& worldComponentData) const = 0;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::world
{

class Entity;
class EntityData;
class IEntityComponent;
class IEntityComponentData;
class IEntityEvent;
class IEntityEventData;

/*! Entity builder interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityBuilder : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create entity from entity data.
	 *
	 * \param entityData Entity data.
	 * \return Concrete entity, null if unable to create entity.
	 */
	virtual Ref< Entity > create(const EntityData* entityData) const = 0;

	/*! Create event from entity event data.
	 *
	 * \param entityEventData Entity event data.
	 * \return Entity event, null if unable to create event.
	 */
	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const = 0;

	/*! Create component from component data.
	 *
	 * \param entityComponentData Entity component data.
	 * \return Component instance, null if unable to create instance.
	 */
	virtual Ref< IEntityComponent > create(const IEntityComponentData* entityComponentData) const = 0;

	/*!
	 */
	template < typename EntityType >
	Ref< EntityType > create(const EntityData* entityData) const
	{
		return checked_type_cast< EntityType*, true >(create(entityData));
	}

	/*!
	 */
	template < typename EntityEventType >
	Ref< EntityEventType > create(const IEntityEventData* entityEventData) const
	{
		return checked_type_cast< EntityEventType*, true >(create(entityEventData));
	}

	/*!
	 */
	template < typename EntityComponentType >
	Ref< EntityComponentType > create(const IEntityComponentData* entityComponentData) const
	{
		return checked_type_cast< EntityComponentType*, true >(create(entityComponentData));
	}
};

}

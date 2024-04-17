/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"

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
class IWorldComponent;
struct UpdateParams;

/*! World container.
 * 
 * The world is a container of all entities representing a world.
 * 
 * \ingroup World
 */
class T_DLLCLASS World : public Object
{
	T_RTTI_CLASS;

public:
	World();

	void destroy();

	/*! Set component in world.
	 *
	 * \param component Component instance.
	 */
	void setComponent(IWorldComponent* component);

	/*! Get component of type.
	 *
	 * \param componentType Type of component.
	 * \return Component instance matching type.
	 */
	IWorldComponent* getComponent(const TypeInfo& componentType) const;

	/*! Get component of type.
	 *
	 * \param ComponentType Type of component.
	 * \return Component instance matching type.
	 */
	template < typename ComponentType >
	ComponentType* getComponent() const
	{
		return checked_type_cast< ComponentType* >(getComponent(type_of< ComponentType >()));
	}

	/*! Get components.
	 *
	 * \return Array of all components.
	 */
	const RefArray< IWorldComponent >& getComponents() const
	{
		return m_components;
	}

	/*! Add entity to world. */
	void addEntity(Entity* entity);

	/*! Remove entity from world. */
	void removeEntity(Entity* entity);

	/*! Check if an entity is part of this world. */
	bool haveEntity(const Entity* entity) const;

	/*! Get an entity by id. */
	Entity* getEntity(const Guid& id) const;

	/*! Get an entity by name, and index if multiple entities are named equally. */
	Entity* getEntity(const std::wstring& name, int32_t index = 0) const;

	/*! Get all entities by name. */
	RefArray< Entity > getEntities(const std::wstring& name) const;

	/*! Get all entities within distance. */
	RefArray< Entity > getEntitiesWithinRange(const Vector4& position, float range) const;

	/*! Update all entities in this world. */
	void update(const UpdateParams& update);

	/*! Get all entities of this world. */
	const RefArray< Entity >& getEntities() const { return m_entities; }

private:
	RefArray< IWorldComponent > m_components;
	RefArray< Entity > m_entities;
	RefArray< Entity > m_deferredAdd;
	RefArray< Entity > m_deferredRemove;
	bool m_update = false;
};

}

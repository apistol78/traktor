/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityComponent;
class World;

/*! World entity class.
 * \ingroup World
 */
class T_DLLCLASS Entity : public Object
{
	T_RTTI_CLASS;

public:
	Entity() = default;

	Entity(const Entity&) = delete;

	explicit Entity(
		const Guid& id,
		const std::wstring_view& name,
		const Transform& transform,
		const EntityState& state = EntityState(),
		const RefArray< IEntityComponent >& components = RefArray< IEntityComponent >()
	);

	/*! Destroy entity resources.
	 *
	 * Called automatically from Entity destructor
	 * in order to destroy any resources allocated
	 * by the entity.
	 *
	 * \note This may be called multiple times for
	 * a single entity so care must be taken when
	 * performing the destruction.
	 */
	virtual void destroy();

	/*! Set world by which entity is part of. */
	void setWorld(World* world);

	/*! Get world by which entity is part of. */
	World* getWorld() const { return m_world; }

	/*! Get entity id. */
	const Guid& getId() const { return m_id; }

	/*! Get entity name. */
	const std::wstring& getName() const { return m_name; }

	/*! Set entity state. */
	EntityState setState(const EntityState& state, const EntityState& mask);

	/*! Get entity state. */
	const EntityState& getState() const { return m_state; }

	/*! Set entity visible state. */
	void setVisible(bool visible) { setState(visible ? EntityState::All : EntityState::None, EntityState::Visible); }

	/*! Check if entity is visible. */
	bool isVisible() const { return m_state.visible; }

	/*! Set entity dynamic state. */
	void setDynamic(bool dynamic) { setState(dynamic ? EntityState::All : EntityState::None, EntityState::Dynamic); }

	/*! Check if entity is dynamic. */
	bool isDynamic() const { return m_state.dynamic; }

	/*! Set entity locked state. */
	void setLocked(bool locked) { setState(locked ? EntityState::All : EntityState::None, EntityState::Locked); }

	/*! Check if entity is locked. */
	bool isLocked() const { return m_state.locked; }

	/*! Set entity transform.
	 *
	 * \param transform Entity transform.
	 */
	virtual void setTransform(const Transform& transform);

	/*! Get entity transform.
	 *
	 * \return Entity transform.
	 */
	virtual Transform getTransform() const;

	/*! Get entity bounding box.
	 * Return entity bounding box in entity space.
	 *
	 * \return Entity bounding box.
	 */
	virtual Aabb3 getBoundingBox() const;

	/*! Update entity.
	 *
	 * \param update Update parameters.
	 */
	virtual void update(const UpdateParams& update);

	/*! Set component in entity.
	 *
	 * \param component Component instance.
	 */
	void setComponent(IEntityComponent* component);

	/*! Get component of type.
	 *
	 * \param componentType Type of component.
	 * \return Component instance matching type.
	 */
	IEntityComponent* getComponent(const TypeInfo& componentType) const;

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
	const RefArray< IEntityComponent >& getComponents() const
	{
		return m_components;
	}

private:
	World* m_world = nullptr;
	Guid m_id;
	std::wstring m_name;
	Transform m_transform = Transform::identity();
	EntityState m_state;
	RefArray< IEntityComponent > m_components;
	const IEntityComponent* m_updating = nullptr;
};

}

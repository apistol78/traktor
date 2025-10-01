/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"

#include "World/IEntityComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.Entity", Entity, Object)

Entity::Entity(
	const Guid& id,
	const std::wstring_view& name,
	const Transform& transform,
	const EntityState& state,
	const RefArray< IEntityComponent >& components)
	: m_id(id)
	, m_name(name)
	, m_transform(transform)
	, m_state(state)
	, m_components(components)
{
	for (auto component : m_components)
	{
		m_updating = component;
		component->setOwner(this);
		component->setState(m_state, EntityState::All, false);
		component->setTransform(m_transform);
	}
	m_updating = nullptr;
}

void Entity::destroy()
{
	T_FATAL_ASSERT(m_world == nullptr);
	for (auto component : m_components)
		component->destroy();
	m_components.clear();
}

void Entity::setWorld(World* world)
{
	m_world = world;
	for (auto component : m_components)
		component->setWorld(world);
}

EntityState Entity::setState(const EntityState& state, const EntityState& mask, bool includeChildren)
{
	const EntityState current = m_state;

	if (mask.visible)
		m_state.visible = state.visible;
	if (mask.dynamic)
		m_state.dynamic = state.dynamic;
	if (mask.locked)
		m_state.locked = state.locked;

	for (auto component : m_components)
		if (component != m_updating)
			component->setState(m_state, mask, includeChildren);

	return current;
}

void Entity::setTransform(const Transform& transform)
{
	m_transform = transform;
	for (auto component : m_components)
		if (component != m_updating)
			component->setTransform(transform);
}

Transform Entity::getTransform() const
{
	return m_transform;
}

Aabb3 Entity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (auto component : m_components)
		boundingBox.contain(component->getBoundingBox());
	return boundingBox;
}

void Entity::update(const UpdateParams& update)
{
	T_FATAL_ASSERT(m_world != nullptr);
	for (auto component : m_components)
	{
		m_updating = component;
		component->update(update);
	}
	m_updating = nullptr;
}

void Entity::setComponent(IEntityComponent* component)
{
	T_FATAL_ASSERT(component);

	component->setOwner(this);
	component->setState(m_state, EntityState::All, false);
	component->setTransform(m_transform);

	// Replace existing component of same type.
	for (size_t i = 0; i < m_components.size(); ++i)
	{
		if (is_type_of(type_of(m_components[i]), type_of(component)))
		{
			m_components[i] = component;
			return;
		}
	}

	// No such component, add last.
	m_components.push_back(component);
}

IEntityComponent* Entity::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
		if (is_type_of(componentType, type_of(component)))
			return component;
	return nullptr;
}

}

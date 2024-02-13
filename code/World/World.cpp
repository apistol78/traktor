/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/EntityEventManager.h"
#include "World/IWorldComponent.h"
#include "World/World.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.World", World, Object)

World::World()
{
	setComponent(new EntityEventManager(512));
}

void World::destroy()
{
	for (auto component : m_components)
		component->destroy();
	m_components.clear();

	for (auto entity : m_entities)
	{
		entity->m_world = nullptr;
		entity->destroy();
	}
	m_entities.clear();
}

void World::setComponent(IWorldComponent* component)
{
	T_FATAL_ASSERT (component);

	// Replace existing component of same type.
	for (auto comp : m_components)
	{
		if (is_type_of(type_of(comp), type_of(component)))
		{
			comp = component;
			return;
		}
	}

	// No such component, add last.
	m_components.push_back(component);
}

IWorldComponent* World::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
	{
		if (is_type_of(componentType, type_of(component)))
			return component;
	}
	return nullptr;
}

void World::addEntity(Entity* entity)
{
	T_FATAL_ASSERT(entity->m_world == nullptr);
	m_entities.push_back(entity);
	entity->m_world = this;
}

void World::removeEntity(Entity* entity)
{
	T_FATAL_ASSERT(entity->m_world == this);
	const bool removed = m_entities.remove(entity);
	T_FATAL_ASSERT(removed);
	entity->m_world = nullptr;
}

bool World::haveEntity(const Entity* entity) const
{
	if (entity->m_world == this)
		return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
	else
		return false;
}

Entity* World::getEntity(const std::wstring& name, int32_t index) const
{
	for (auto entity : m_entities)
	{
		if (entity->getName() == name)
		{
			if (index-- <= 0)
				return entity;
		}
	}
	return nullptr;
}

RefArray< Entity > World::getEntities(const std::wstring& name) const
{
	RefArray< Entity > entities;
	for (auto entity : m_entities)
	{
		if (entity->getName() == name)
			entities.push_back(entity);
	}
	return entities;
}

RefArray< Entity > World::getEntitiesWithinRange(const Vector4& position, float range) const
{
	RefArray< Entity > entities;
	for (auto entity : m_entities)
	{
		const Scalar distance = (entity->getTransform().translation() - position).xyz0().length();
		if (distance <= range)
			entities.push_back(entity);
	}
	return entities;
}

void World::update(const UpdateParams& update)
{
	for (auto component : m_components)
		component->update(this, update);

	RefArray< Entity > entities = m_entities;
	for (auto entity : entities)
		entity->update(update);
}

}

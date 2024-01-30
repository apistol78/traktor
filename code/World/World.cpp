/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/World.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.World", World, Object)

void World::destroy()
{
	for (auto entity : m_entities)
		entity->destroy();
	m_entities.clear();
}

void World::addEntity(Entity* entity)
{
	m_entities.push_back(entity);
}

void World::removeEntity(Entity* entity)
{
	m_entities.remove(entity);
}

bool World::haveEntity(const Entity* entity) const
{
	return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
}

Entity* World::getEntity(const std::wstring_view& name, int32_t index) const
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

RefArray< Entity > World::getEntities(const std::wstring_view& name) const
{
	RefArray< Entity > entities;
	for (auto entity : m_entities)
	{
		if (entity->getName() == name)
			entities.push_back(entity);
	}
	return entities;
}

void World::update(const UpdateParams& update)
{
	for (auto entity : m_entities)
		entity->update(update);
}

}

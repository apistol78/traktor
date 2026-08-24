/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/World.h"

#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Render/IRenderSystem.h"
#include "World/Entity.h"
#include "World/Entity/CullingComponent.h"
#include "World/Entity/EventManagerComponent.h"
#include "World/Entity/IrradianceGridComponent.h"
#include "World/Entity/RTWorldComponent.h"
#include "World/IWorldComponent.h"

#define T_USE_UPDATE_JOBS

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.World", World, Object)

World::World(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	setComponent(new CullingComponent(resourceManager, renderSystem));
	setComponent(new EventManagerComponent(512));
	setComponent(new IrradianceGridComponent());
	if (renderSystem->supportRayTracing())
		setComponent(new RTWorldComponent(renderSystem));
}

void World::destroy()
{
	T_FATAL_ASSERT(m_deferredAdd.empty());
	T_FATAL_ASSERT(m_deferredRemove.empty());

	for (auto entity : m_entities)
	{
		entity->setWorld(nullptr);
		entity->destroy();
	}
	m_entities.clear();

	for (auto component : m_components)
		component->destroy();
	m_components.clear();
}

void World::setComponent(IWorldComponent* component)
{
	T_FATAL_ASSERT(component);

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

bool World::removeComponent(IWorldComponent* component)
{
	return m_components.remove(component);
}

IWorldComponent* World::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
		if (is_type_of(componentType, type_of(component)))
			return component;
	return nullptr;
}

void World::addEntity(Entity* entity)
{
	if (entity->getWorld() != nullptr)
		return;
	if (m_update)
		m_deferredAdd.push_back(entity);
	else
		m_entities.push_back(entity);
	entity->setWorld(this);
}

void World::removeEntity(Entity* entity)
{
	if (entity->getWorld() != this)
		return;
	if (m_update)
		m_deferredRemove.push_back(entity);
	else
	{
		const bool removed = m_entities.remove(entity);
		T_FATAL_ASSERT(removed);
	}
	entity->setWorld(nullptr);
}

bool World::haveEntity(const Entity* entity) const
{
	if (entity->getWorld() == this)
		return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
	else
		return false;
}

Entity* World::getEntity(const Guid& id) const
{
	for (auto entity : m_entities)
		if (entity->getId() == id)
			return entity;
	return nullptr;
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
		if (entity->getName() == name)
			entities.push_back(entity);
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

RefArray< Entity > World::getEntitiesWithinRange(const std::wstring& name, const Vector4& position, float range) const
{
	RefArray< Entity > entities;
	for (auto entity : m_entities)
	{
		if (entity->getName() == name)
		{
			const Scalar distance = (entity->getTransform().translation() - position).xyz0().length();
			if (distance <= range)
				entities.push_back(entity);
		}
	}
	return entities;
}

void World::update(const UpdateParams& update)
{
	// Update all world components.
	for (auto component : m_components)
		component->update(this, update);

	// Update all entities.
	m_update = true;

#if defined(T_USE_UPDATE_JOBS)
	AlignedVector< Entity* > concurrent;
	concurrent.reserve(m_entities.size());

	for (auto entity : m_entities)
	{
		if (entity->getWorld() != nullptr && entity->needConcurrentUpdate())
			concurrent.push_back(entity);
	}

	if (!concurrent.empty())
	{
		const uint32_t c_minEntitiesPerChunk = 8;
		const uint32_t count = (uint32_t)concurrent.size();
		const uint32_t workers = JobManager::getInstance().getWorkerCount();
		const uint32_t byWork = (count + c_minEntitiesPerChunk - 1) / c_minEntitiesPerChunk;

		uint32_t chunks = (workers > 0) ? workers : 1;
		if (byWork < chunks)
			chunks = byWork;

		AlignedVector< Job::task_t > jobs;
		jobs.reserve(chunks);

		for (uint32_t i = 0; i < chunks; ++i)
		{
			const uint32_t begin = (count * i) / chunks;
			const uint32_t end = (count * (i + 1)) / chunks;
			jobs.push_back([&concurrent, &update, begin, end](){
				for (uint32_t j = begin; j < end; ++j)
					concurrent[j]->update(update, true);
			});
		}

		JobManager::getInstance().fork(jobs.c_ptr(), jobs.size());
	}

	for (auto entity : m_entities)
	{
		if (entity->getWorld() != nullptr)
			entity->update(update, false);
	}
#else
	for (auto entity : m_entities)
	{
		if (entity->getWorld() != nullptr)
		{
			entity->update(update, true);
			entity->update(update, false);
		}
	}
#endif

	m_update = false;

	// Add entities which has been added during entity update.
	if (!m_deferredAdd.empty())
	{
		m_entities.insert(m_entities.end(), m_deferredAdd.begin(), m_deferredAdd.end());
		m_deferredAdd.resize(0);
	}

	// Remove entities which has been removed during entity update.
	if (!m_deferredRemove.empty())
	{
		for (auto entity : m_deferredRemove)
		{
			const bool removed = m_entities.remove(entity);
			T_FATAL_ASSERT(removed);
		}
		m_deferredRemove.resize(0);
	}
}

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/EntityStreamingComponent.h"

#include "Core/Thread/JobManager.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/World.h"

#include <limits>

namespace traktor::world
{
namespace
{

const int32_t c_maxUnloadsPerUpdate = 2;

Scalar distanceToCell(const Vector4& focus, const Aabb3& boundingBox)
{
	const Vector4 closest = max(min(focus, boundingBox.mx), boundingBox.mn);
	return (focus - closest).xyz0().length();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityStreamingComponent", EntityStreamingComponent, IWorldComponent)

EntityStreamingComponent::EntityStreamingComponent(const IEntityBuilder* entityBuilder, resource::IResourceManager* resourceManager, const EntityStreamingComponentData* data)
	: m_entityBuilder(entityBuilder)
	, m_resourceManager(resourceManager)
	, m_data(data)
{
	m_cells.resize(data->getCells().size());
	for (size_t i = 0; i < m_cells.size(); ++i)
		m_cells[i].data = &data->getCells()[i];
}

void EntityStreamingComponent::destroy()
{
	// Entities added to world are destroyed by world; only pending entities remain.
	if (m_job)
	{
		m_cancel = true;
		m_job->wait();
		m_job = nullptr;
		for (auto entity : m_loadingEntities)
			entity->destroy();
		m_loadingEntities.clear();
	}
	m_cells.clear();
	m_data = nullptr;
	m_resourceManager = nullptr;
	m_entityBuilder = nullptr;
}

void EntityStreamingComponent::setWorld(World* world)
{
	m_world = world;
}

void EntityStreamingComponent::update(const UpdateParams& update)
{
	if (!m_haveFocus)
		return;

	// Swap in entities of loaded cell, or discard them if level is no longer wanted.
	bool unloaded = processPendingJob(0);

	// Unload cells beyond reach and find nearest cell which need another level.
	Scalar nearestDistance(std::numeric_limits< float >::max());
	int32_t nearestCell = -1;
	Level nearestLevel = Level::None;
	int32_t unloadCount = 0;

	for (int32_t i = 0; i < (int32_t)m_cells.size(); ++i)
	{
		// Cell being loaded is left alone until its job has finished.
		if (i == m_loadingCell)
			continue;

		Cell& cell = m_cells[i];
		const Scalar distance = distanceToCell(m_focus, cell.data->boundingBox);
		const Level level = wantedLevel(cell, distance);
		if (level == cell.level)
			continue;

		if (level == Level::None)
		{
			if (unloadCount < c_maxUnloadsPerUpdate)
			{
				RefArray< Entity > none;
				unloaded |= replaceEntities(cell, Level::None, none);
				++unloadCount;
			}
		}
		else if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestCell = i;
			nearestLevel = level;
		}
	}

	// Create entities of one cell at a time in background.
	if (!m_job && nearestCell >= 0)
	{
		const Cell& cell = m_cells[nearestCell];
		m_loadingCell = nearestCell;
		m_loadingLevel = nearestLevel;
		m_job = JobManager::getInstance().add([this, &cell]() {
			createEntities(cell, m_loadingLevel, m_loadingEntities);
		});
	}

	// Release resources no longer used by any entity.
	if (unloaded)
		m_resourceManager->unloadUnusedResident();
}

void EntityStreamingComponent::load(const Vector4& focus)
{
	processPendingJob(-1);
	setFocus(focus);

	for (auto& cell : m_cells)
	{
		const Level level = wantedLevel(cell, distanceToCell(m_focus, cell.data->boundingBox));
		if (level == cell.level)
			continue;

		RefArray< Entity > entities;
		createEntities(cell, level, entities);
		replaceEntities(cell, level, entities);
	}
}

void EntityStreamingComponent::setFocus(const Vector4& focus)
{
	m_focus = focus;
	m_haveFocus = true;
}

EntityStreamingComponent::Level EntityStreamingComponent::wantedLevel(const Cell& cell, const Scalar& distance) const
{
	// Current level is kept until beyond its unload distance.
	if (distance < Scalar(m_data->getLoadDistance()) || (cell.level == Level::Full && distance <= Scalar(m_data->getUnloadDistance())))
		return Level::Full;

	if (cell.data->hlodEntityData.empty())
		return Level::None;

	if (distance < Scalar(m_data->getHlodLoadDistance()) || (cell.level != Level::None && distance <= Scalar(m_data->getHlodUnloadDistance())))
		return Level::Hlod;

	return Level::None;
}

bool EntityStreamingComponent::replaceEntities(Cell& cell, Level level, RefArray< Entity >& entities)
{
	const bool unloaded = (cell.level == Level::Full && !cell.entities.empty());

	for (auto entity : cell.entities)
	{
		// Entity no longer in world has been taken by others and is theirs.
		if (entity->getWorld() != m_world)
		{
			cell.removed.insert(entity->getId());
			continue;
		}
		m_world->removeEntity(entity);
		entity->destroy();
	}

	for (auto entity : entities)
		m_world->addEntity(entity);

	cell.entities.swap(entities);
	cell.level = level;
	entities.clear();
	return unloaded;
}

bool EntityStreamingComponent::processPendingJob(int32_t timeout)
{
	if (!m_job)
		return false;

	if (!m_job->wait(timeout))
		return false;

	bool unloaded = false;

	Cell& cell = m_cells[m_loadingCell];
	if (wantedLevel(cell, distanceToCell(m_focus, cell.data->boundingBox)) == m_loadingLevel)
		unloaded = replaceEntities(cell, m_loadingLevel, m_loadingEntities);

	for (auto entity : m_loadingEntities)
		entity->destroy();
	m_loadingEntities.clear();

	m_job = nullptr;
	m_loadingCell = -1;
	m_loadingLevel = Level::None;
	return unloaded;
}

void EntityStreamingComponent::createEntities(const Cell& cell, Level level, RefArray< Entity >& outEntities) const
{
	const auto& entityData = (level == Level::Full) ? cell.data->entityData : cell.data->hlodEntityData;
	for (auto ed : entityData)
	{
		if (m_cancel)
			break;

		if (cell.removed.find(ed->getId()) != cell.removed.end())
			continue;

		Ref< Entity > entity = m_entityBuilder->create(ed);
		if (entity)
			outEntities.push_back(entity);
	}
}

}

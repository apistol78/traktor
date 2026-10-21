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
		for (auto entity : m_cells[m_loadingCell].entities)
			entity->destroy();
	}
	m_cells.clear();
	m_data = nullptr;
	m_resourceManager = nullptr;
	m_entityBuilder = nullptr;
}

void EntityStreamingComponent::update(World* world, const UpdateParams& update)
{
	if (!m_haveFocus)
		return;

	const Scalar loadDistance(m_data->getLoadDistance());
	const Scalar unloadDistance(m_data->getUnloadDistance());

	// Cells initially in range are created immediately, as if they were part of scene.
	if (!m_primed)
	{
		for (auto& cell : m_cells)
		{
			if (distanceToCell(m_focus, cell.data->boundingBox) >= loadDistance)
				continue;
			createEntities(cell);
			for (auto entity : cell.entities)
				world->addEntity(entity);
			cell.loaded = true;
		}
		m_primed = true;
		return;
	}

	// Add entities of loaded cell to world, or discard them if cell is no longer wanted.
	if (m_job && m_job->wait(0))
	{
		Cell& cell = m_cells[m_loadingCell];
		if (distanceToCell(m_focus, cell.data->boundingBox) <= unloadDistance)
		{
			for (auto entity : cell.entities)
				world->addEntity(entity);
			cell.loaded = true;
		}
		else
		{
			for (auto entity : cell.entities)
				entity->destroy();
			cell.entities.clear();
		}
		m_job = nullptr;
		m_loadingCell = -1;
	}

	// Unload cells beyond unload distance and find nearest cell to load.
	Scalar nearestDistance = loadDistance;
	int32_t nearestCell = -1;
	int32_t unloadCount = 0;

	for (int32_t i = 0; i < (int32_t)m_cells.size(); ++i)
	{
		Cell& cell = m_cells[i];
		const Scalar distance = distanceToCell(m_focus, cell.data->boundingBox);
		if (cell.loaded)
		{
			if (distance > unloadDistance && unloadCount < c_maxUnloadsPerUpdate)
			{
				for (auto entity : cell.entities)
				{
					// Entity no longer in world has been taken by others and is theirs.
					if (entity->getWorld() != world)
					{
						cell.removed.insert(entity->getId());
						continue;
					}
					world->removeEntity(entity);
					entity->destroy();
				}
				cell.entities.clear();
				cell.loaded = false;
				++unloadCount;
			}
		}
		else if (i != m_loadingCell && distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestCell = i;
		}
	}

	// Create entities of one cell at a time in background.
	if (!m_job && nearestCell >= 0)
	{
		Cell& cell = m_cells[nearestCell];
		m_loadingCell = nearestCell;
		m_job = JobManager::getInstance().add([this, &cell]() {
			createEntities(cell);
		});
	}

	// Release resources no longer used by any entity.
	if (unloadCount > 0)
		m_resourceManager->unloadUnusedResident();
}

void EntityStreamingComponent::setFocus(const Vector4& focus)
{
	m_focus = focus;
	m_haveFocus = true;
}

void EntityStreamingComponent::createEntities(Cell& cell) const
{
	for (auto entityData : cell.data->entityData)
	{
		if (m_cancel)
			break;

		if (cell.removed.find(entityData->getId()) != cell.removed.end())
			continue;

		Ref< Entity > entity = m_entityBuilder->create(entityData);
		if (entity)
			cell.entities.push_back(entity);
	}
}

}

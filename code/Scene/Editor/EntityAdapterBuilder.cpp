/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/DeepHash.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/ExternalEntityBuilder.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityFactory.h"
#include "World/Editor/EditorAttributesComponentData.h"

namespace traktor::scene
{
	namespace
	{

void collectAllAdapters(EntityAdapter* entityAdapter, RefArray< EntityAdapter >& outEntityAdapters)
{
	if (!entityAdapter)
		return;

	outEntityAdapters.push_back(entityAdapter);

	for (auto childAdapter : entityAdapter->getChildren())
		collectAllAdapters(childAdapter, outEntityAdapters);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterBuilder", EntityAdapterBuilder, world::IEntityBuilder)

EntityAdapterBuilder::EntityAdapterBuilder(
	SceneEditorContext* context,
	world::IEntityBuilder* entityBuilder,
	EntityAdapter* currentEntityAdapter
)
:	m_context(context)
,	m_entityBuilder(new ExternalEntityBuilder(entityBuilder))
,	m_cacheHit(0)
,	m_cacheMiss(0)
{
	RefArray< EntityAdapter > entityAdapters;
	collectAllAdapters(currentEntityAdapter, entityAdapters);

	for (auto entityAdapter : entityAdapters)
	{
		if (entityAdapter->getEntityData())
		{
			Cache& cache = m_cache[&type_of(entityAdapter->getEntityData())];
			cache.adapters.push_back(entityAdapter);

			if (
				entityAdapter->getHash() &&
				entityAdapter->getEntity() &&
				entityAdapter->getChildren().empty()
			)
			{
				uint32_t hash = entityAdapter->getHash();
				cache.leafEntities[hash].push_back(entityAdapter->getEntity());
			}
		}
	}

	T_ASSERT(!m_rootAdapter);
}

EntityAdapterBuilder::~EntityAdapterBuilder()
{
	// Ensure all unused entities from cache is properly destroyed.
	for (auto ca : m_cache)
	{
		for (auto ue : ca.second.leafEntities)
		{
			for (auto e : ue.second)
			{
				if (e)
					e->destroy();
			}
		}
	}
}

void EntityAdapterBuilder::addFactory(const world::IEntityFactory* entityFactory)
{
	T_FATAL_ERROR;
}

void EntityAdapterBuilder::removeFactory(const world::IEntityFactory* entityFactory)
{
	T_FATAL_ERROR;
}

const world::IEntityFactory* EntityAdapterBuilder::getFactory(const world::EntityData* entityData) const
{
	return m_entityBuilder->getFactory(entityData);
}

const world::IEntityFactory* EntityAdapterBuilder::getFactory(const world::IEntityEventData* entityEventData) const
{
	return m_entityBuilder->getFactory(entityEventData);
}

const world::IEntityFactory* EntityAdapterBuilder::getFactory(const world::IEntityComponentData* entityComponentData) const
{
	return m_entityBuilder->getFactory(entityComponentData);
}

Ref< world::Entity > EntityAdapterBuilder::create(const world::EntityData* entityData) const
{
	Ref< EntityAdapter > entityAdapter;
	Ref< world::Entity > entity;

	if (!entityData)
		return nullptr;

	// Calculate deep hash of entity data.
	const uint32_t hash = DeepHash(entityData).get();

	// Get adapter; reuse adapters containing same type of entity.
	Cache& cache = m_cache[&type_of(entityData)];
	if (!cache.adapters.empty())
	{
		entityAdapter = cache.adapters.front();
		T_FATAL_ASSERT (entityAdapter != nullptr);
		cache.adapters.pop_front();
		T_FATAL_ASSERT (&type_of(entityAdapter->getEntityData()) == &type_of(entityData));
		entityAdapter->unlinkFromParent();
	}
	else
	{
		entityAdapter = new EntityAdapter(m_context);

		// Get visibility state from editor attributes, do this
		// only when a new adapter is created as we want to keep
		// editing session state.
		if (auto editorAttributes = entityData->getComponent< world::EditorAttributesComponentData >())
		{
			entityAdapter->setVisible(editorAttributes->visible);
			entityAdapter->setLocked(editorAttributes->locked);
		}
	}

	// Setup relationship with parent.
	if (m_currentAdapter)
		m_currentAdapter->link(entityAdapter);
	else
	{
		T_FATAL_ASSERT (m_rootAdapter == nullptr);
		m_rootAdapter = entityAdapter;
		T_FATAL_ASSERT (m_rootAdapter->getParent() == nullptr);
	}

	// Re-use leaf entities if hash match.
	if (!cache.leafEntities.empty())
	{
		RefArray< world::Entity >& entities = cache.leafEntities[hash];
		if (!entities.empty())
		{
			entity = entities.front();
			entities.pop_front();
			m_cacheHit++;
		}
	}

	// If no leaf entity then we need to re-create the entity.
	if (!entity)
	{
		// Unlink all children first; new children will be added recursively.
		entityAdapter->unlinkAllChildren();
		T_FATAL_ASSERT (entityAdapter->getChildren().empty());

		// Create the concrete entity.
		{
			T_ANONYMOUS_VAR(Save< Ref< EntityAdapter > >)(m_currentAdapter, entityAdapter);
			const world::IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityData);
			T_FATAL_ASSERT (entityFactory);
			entity = entityFactory->createEntity(this, *entityData);
		}

		// If still no entity then we create a null placeholder.
		if (!entity)
		{
			log::debug << L"Unable to create entity from \"" << type_name(entityData) << L"\"; using empty entity as placeholder." << Endl;
			entity = new world::Entity(entityData->getName(), entityData->getTransform());
		}

		m_cacheMiss++;
	}

	T_FATAL_ASSERT (entity);
	entity->setTransform(entityData->getTransform());

	// Prepare entity adapter.
	entityAdapter->prepare(
		const_cast< world::EntityData* >(entityData),
		entity,
		hash
	);

	return entity;
}

Ref< world::IEntityEvent > EntityAdapterBuilder::create(const world::IEntityEventData* entityEventData) const
{
	const world::IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityEventData);
	if (entityFactory)
		return entityFactory->createEntityEvent(this, *entityEventData);
	else
		return nullptr;
}

Ref< world::IEntityComponent > EntityAdapterBuilder::create(const world::IEntityComponentData* entityComponentData) const
{
	const world::IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityComponentData);
	if (entityFactory)
		return entityFactory->createEntityComponent(this, *entityComponentData);
	else
		return nullptr;
}

const world::IEntityBuilder* EntityAdapterBuilder::getCompositeEntityBuilder() const
{
	return m_entityBuilder;
}

}

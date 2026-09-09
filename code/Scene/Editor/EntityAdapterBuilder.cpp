/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Editor/Traverser.h"
#include "World/Entity.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/IEntityComponent.h"
#include "World/IEntityFactory.h"
#include "World/World.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/ExternalEntityData.h"

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

bool isComponentCacheable(const world::IEntityComponentData* componentData)
{
	return !is_a< world::GroupComponentData >(componentData);
}

void addToWorld(world::World* world, world::Entity* entity)
{
	// Entity might belong to a replaced world when world has been rebuilt.
	if (entity->getWorld() != nullptr && entity->getWorld() != world)
		entity->getWorld()->removeEntity(entity);

	world->addEntity(entity);

	if (auto group = entity->getComponent< world::GroupComponent >())
		for (auto childEntity : group->getEntities())
			addToWorld(world, childEntity);
}

void resetFromData(EntityAdapter* entityAdapter)
{
	for (auto child : entityAdapter->getChildren())
		resetFromData(child);

	world::Entity* entity = entityAdapter->getEntity();
	if (entity != nullptr)
	{
		entity->setTransform(entityAdapter->getEntityData()->getTransform());
		entity->setState(entityAdapter->getEntityData()->getState(), world::EntityState::All, false);
	}
}

void rebindEntityData(EntityAdapter* entityAdapter, world::EntityData* entityData)
{
	SmallMap< Guid, EntityAdapter* > adapters;
	RefArray< EntityAdapter > descendants;
	collectAllAdapters(entityAdapter, descendants);
	for (auto adapter : descendants)
		adapters[adapter->getId()] = adapter;

	entityAdapter->prepare(entityData, entityAdapter->getEntity());
	world::Traverser::visit((const ISerializable*)entityData, [&](const world::EntityData* childData) {
		auto it = adapters.find(childData->getId());
		if (it != adapters.end())
			it->second->prepare(const_cast< world::EntityData* >(childData), it->second->getEntity());
		return world::Traverser::Result::Continue;
	});
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterBuilder", EntityAdapterBuilder, world::IEntityBuilder)

EntityAdapterBuilder::EntityAdapterBuilder(
	SceneEditorContext* context,
	const world::IEntityFactory* entityFactory,
	world::World* world,
	EntityAdapter* currentEntityAdapter
)
:	m_context(context)
,	m_entityFactory(entityFactory)
,	m_world(world)
{
	RefArray< EntityAdapter > entityAdapters;
	collectAllAdapters(currentEntityAdapter, entityAdapters);
	for (auto entityAdapter : entityAdapters)
	{
		if (
			entityAdapter->getEntityData() &&
			entityAdapter->getEntityData()->getId().isNotNull()
		)
		{
			Cache& cache = m_cache[entityAdapter->getEntityData()->getId()];
			cache.adapter = entityAdapter;
		}
	}
}

Ref< world::Entity > EntityAdapterBuilder::create(const world::EntityData* entityData) const
{
	Ref< EntityAdapter > entityAdapter;

	if (!entityData)
		return nullptr;

	// Do not create adapters when we're inside an external entity.
	if (is_a< world::ExternalEntityData >(m_currentEntityData))
	{
		const world::EntityBuilder entityBuilder(m_entityFactory, m_world);
		return m_entityFactory->createEntity(&entityBuilder, *entityData);
	}

	const uint32_t hash = DeepHash(entityData).get();

	// Get adapter; reuse adapters containing same type of entity.
	Cache& cache = m_cache[entityData->getId()];
	if (cache.adapter != nullptr)
	{
		entityAdapter = cache.adapter;
		T_FATAL_ASSERT (entityAdapter != nullptr);
		T_FATAL_ASSERT (&type_of(entityAdapter->getEntityData()) == &type_of(entityData));
		entityAdapter->unlinkFromParent();
	}
	else
		entityAdapter = new EntityAdapter(m_context);

	// Setup relationship with parent.
	if (m_currentAdapter)
		m_currentAdapter->link(nullptr, entityAdapter);
	else
	{
		T_FATAL_ASSERT (m_rootAdapter == nullptr);
		m_rootAdapter = entityAdapter;
		T_FATAL_ASSERT (m_rootAdapter->getParent() == nullptr);
	}

	// Reuse entire entity, including children, if data is unchanged since last build.
	if (entityAdapter->getEntity() != nullptr && hash != 0 && entityAdapter->getEntityProductHash() == hash)
	{
		// Undo/redo replaces data instances; re-bind adapters to new instances.
		if (entityAdapter->getEntityData() != entityData)
			rebindEntityData(entityAdapter, const_cast< world::EntityData* >(entityData));

		addToWorld(m_world, entityAdapter->getEntity());
		resetFromData(entityAdapter);

		return entityAdapter->getEntity();
	}

	// Unlink all children first; new children will be added recursively.
	entityAdapter->unlinkAllChildren();
	T_FATAL_ASSERT (entityAdapter->getChildren().empty());

	// Make all previous components orphans.
	if (entityAdapter->getEntity() != nullptr)
	{
		for (auto component : entityAdapter->getComponents())
			component->setOwner(nullptr);
	}

	// Create the concrete entity.
	Ref< world::Entity > entity;
	{
		T_ANONYMOUS_VAR(Save< Ref< EntityAdapter > >)(m_currentAdapter, entityAdapter);
		T_ANONYMOUS_VAR(Save< Ref< const world::EntityData > >)(m_currentEntityData, entityData);
		entity = m_entityFactory->createEntity(this, *entityData);
	}

	uint32_t entityProductHash = hash;

	// If still no entity then we create a null placeholder.
	if (!entity)
	{
		log::debug << L"Unable to create entity from \"" << type_name(entityData) << L"\"; using empty entity as placeholder." << Endl;
		entity = new world::Entity(
			entityData->getId(),
			entityData->getName(),
			entityData->getTransform(),
			entityData->getState()
		);

		// Do not cache placeholders; retry build every time.
		entityProductHash = 0;
	}

	entity->setTransform(entityData->getTransform());

	// Add entity to world container.
	m_world->addEntity(entity);

	// Prepare entity adapter.
	entityAdapter->prepare(
		const_cast< world::EntityData* >(entityData),
		entity
	);
	entityAdapter->setEntityProductHash(entityProductHash);

	return entity;
}

Ref< world::IEntityEvent > EntityAdapterBuilder::create(const world::IEntityEventData* entityEventData) const
{
	return m_entityFactory->createEntityEvent(this, *entityEventData);
}

Ref< world::IEntityComponent > EntityAdapterBuilder::create(const world::IEntityComponentData* entityComponentData) const
{
	Ref< world::IEntityComponent > entityComponent;

	// Try to find existing component.
	entityComponent = m_currentAdapter->findComponentProduct(entityComponentData);
	if (entityComponent)
		return entityComponent;

	// Create component through factory.
	entityComponent = m_entityFactory->createEntityComponent(this, *entityComponentData);
	if (!entityComponent)
		return nullptr;

	// Cache component in adapter so we can reuse it later.
	if (isComponentCacheable(entityComponentData))
	{
		m_currentAdapter->setComponentProduct(
			entityComponentData,
			entityComponent
		);
	}

	return entityComponent;
}

Ref< world::IWorldComponent > EntityAdapterBuilder::create(const world::IWorldComponentData* worldComponentData) const
{
	return m_entityFactory->createWorldComponent(this, *worldComponentData);
}

}

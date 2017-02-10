#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/DeepHash.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityFactory.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

void collectAllAdapters(EntityAdapter* entityAdapter, RefArray< EntityAdapter >& outEntityAdapters)
{
	if (!entityAdapter)
		return;

	outEntityAdapters.push_back(entityAdapter);

	const RefArray< EntityAdapter >& childAdapters = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = childAdapters.begin(); i != childAdapters.end(); ++i)
		collectAllAdapters(*i, outEntityAdapters);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterBuilder", EntityAdapterBuilder, world::IEntityBuilder)

EntityAdapterBuilder::EntityAdapterBuilder(
	SceneEditorContext* context,
	world::IEntityBuilder* entityBuilder,
	EntityAdapter* currentEntityAdapter
)
:	m_context(context)
,	m_entityBuilder(entityBuilder)
{
	RefArray< EntityAdapter > entityAdapters;
	collectAllAdapters(currentEntityAdapter, entityAdapters);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		EntityAdapter* entityAdapter = *i;
		T_FATAL_ASSERT (entityAdapter);

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

	T_ASSERT (!m_rootAdapter);
}

EntityAdapterBuilder::~EntityAdapterBuilder()
{
}

void EntityAdapterBuilder::addFactory(const world::IEntityFactory* entityFactory)
{
	m_entityBuilder->addFactory(entityFactory);
}

void EntityAdapterBuilder::removeFactory(const world::IEntityFactory* entityFactory)
{
	m_entityBuilder->removeFactory(entityFactory);
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
	uint32_t hash;

	if (!entityData)
		return 0;

	Cache& cache = m_cache[&type_of(entityData)];

	// Get adapter; reuse adapters containing same type of entity.
	if (!cache.adapters.empty())
	{
		entityAdapter = cache.adapters.front();
		T_FATAL_ASSERT (entityAdapter != 0);
		cache.adapters.pop_front();
		T_FATAL_ASSERT (&type_of(entityAdapter->getEntityData()) == &type_of(entityData));
		entityAdapter->unlinkFromParent();
	}
	else
	{
		entityAdapter = new EntityAdapter(m_context);

		// Get visibility state from layer entity data, do this
		// only when a new adapter is created as we want to keep
		// editing session state.
		if (const world::LayerEntityData* layerEntityData = dynamic_type_cast< const world::LayerEntityData* >(entityData))
		{
			entityAdapter->setVisible(layerEntityData->isVisible());
			entityAdapter->setLocked(layerEntityData->isLocked());
		}
	}

	// Setup relationship with parent.
	if (m_currentAdapter)
		m_currentAdapter->link(entityAdapter);
	else
	{
		T_FATAL_ASSERT (m_rootAdapter == 0);
		m_rootAdapter = entityAdapter;
		T_FATAL_ASSERT (m_rootAdapter->getParent() == 0);
	}

	// Re-use leaf entities if hash match.
	if (!cache.leafEntities.empty())
	{
		hash = DeepHash(entityData).get();
		RefArray< world::Entity >& entities = cache.leafEntities[hash];
		if (!entities.empty())
		{
			entity = entities.front();
			entities.pop_front();
		}
	}
	else
		hash = 0;

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
			log::debug << L"Unable to create entity from \"" << type_name(entityData) << L"\"; using empty entity as placeholder" << Endl;
			entity = new world::ComponentEntity(entityData->getTransform());
		}
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
	return m_entityBuilder->create(entityEventData);
}

Ref< world::IEntityComponent > EntityAdapterBuilder::create(const world::IEntityComponentData* entityComponentData) const
{
	return m_entityBuilder->create(entityComponentData);
}

const world::IEntityBuilder* EntityAdapterBuilder::getCompositeEntityBuilder() const
{
	return m_entityBuilder->getCompositeEntityBuilder();
}

EntityAdapter* EntityAdapterBuilder::getRootAdapter() const
{
	return m_rootAdapter;
}

	}
}

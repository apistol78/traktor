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
#include "World/Entity/GroupEntity.h"
#include "World/Entity/NullEntity.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

Ref< IEntityEditor > createEntityEditor(
	SceneEditorContext* context,
	const RefArray< const IEntityEditorFactory >& entityEditorFactories,
	EntityAdapter* entityAdapter
)
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	const IEntityEditorFactory* entityEditorFactory = 0;

	const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());

	for (RefArray< const IEntityEditorFactory >::const_iterator i = entityEditorFactories.begin(); i != entityEditorFactories.end(); ++i)
	{
		TypeInfoSet entityDataTypes = (*i)->getEntityDataTypes();
		for (TypeInfoSet::const_iterator j = entityDataTypes.begin(); j != entityDataTypes.end(); ++j)
		{
			if (is_type_of(**j, entityDataType))
			{
				uint32_t classDifference = type_difference(**j, entityDataType);
				if (classDifference < minClassDifference)
				{
					entityEditorFactory = *i;
					minClassDifference = classDifference;
				}
			}
		}
	}

	if (entityEditorFactory)
	{
		Ref< IEntityEditor > entityEditor = entityEditorFactory->createEntityEditor(context, entityAdapter);
		T_ASSERT_M (entityEditor, L"Entity editor factory returned null");
		return entityEditor;
	}
	else
		return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterBuilder", EntityAdapterBuilder, world::IEntityBuilder)

EntityAdapterBuilder::EntityAdapterBuilder(
	SceneEditorContext* context,
	world::IEntityBuilder* entityBuilder,
	const RefArray< const IEntityEditorFactory >& entityEditorFactories
)
:	m_context(context)
,	m_entityBuilder(entityBuilder)
,	m_entityEditorFactories(entityEditorFactories)
,	m_adapterCount(0)
{
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters, SceneEditorContext::GfDescendants);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		Ref< EntityAdapter > entityAdapter = *i;

		// Get cache entry from type of entity data.
		Cache* cache = 0;
		if (entityAdapter->getEntityData())
		{
			const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());
			cache = &m_cache[&entityDataType];
		}

		// Cache leaf entities; else just remove from adapter.
		Ref< world::Entity > entity = entityAdapter->getEntity();
		if (entity)
		{
			if (
				cache &&
				entityAdapter->getChildren().empty()
			)
			{
				const world::EntityData* entityData = entityAdapter->getEntityData();
				T_FATAL_ASSERT (entityData);

				uint32_t hash = DeepHash(entityData).get();
				cache->leafEntities[hash].push_back(entity);
			}

			entityAdapter->setEntity(0);
		}

		// Unlink adapter from parent.
		EntityAdapter* parent = entityAdapter->getParent();
		if (parent)
			parent->unlink(entityAdapter);

		// Save adapter for easy re-use.
		if (cache)
			cache->adapters.push_back(entityAdapter);

		// Release entity data reference.
		entityAdapter->setEntityData(0);
	}

	m_adapterCount = 0;

	T_ASSERT (!m_rootAdapter);
}

EntityAdapterBuilder::~EntityAdapterBuilder()
{
	log::debug << L"Entity builder performance" << Endl;
	log::debug << IncreaseIndent;

	double totalTime = 0.0;
	for (std::map< const TypeInfo*, std::pair< int32_t, double > >::const_iterator i = m_buildTimes.begin(); i != m_buildTimes.end(); ++i)
	{
		log::debug << i->first->getName() << L" : " << int32_t(i->second.second * 1000.0) << L" ms in " << i->second.first << L" count(s) (" << int32_t(i->second.second * 100000.0 / i->second.first) / 100.0 << L" ms/call)" << Endl;
		totalTime += i->second.second;
	}

	log::debug << L"Total : " << int32_t(totalTime * 1000.0) << L" ms" << Endl;
	log::debug << DecreaseIndent;
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

Ref< world::Entity > EntityAdapterBuilder::create(const world::EntityData* entityData) const
{
	Ref< EntityAdapter > entityAdapter;

	if (!entityData)
		return 0;

	Cache& cache = m_cache[&type_of(entityData)];

	// Get adapter; reuse adapters containing same type of entity.
	if (!cache.adapters.empty())
	{
		entityAdapter = cache.adapters.front();
		cache.adapters.pop_front();
	}
	else
		entityAdapter = new EntityAdapter();

	// Setup relationship with parent.
	if (m_currentAdapter)
		m_currentAdapter->link(entityAdapter);
	else
		m_rootAdapter = entityAdapter;

	// Find entity factory.
	Ref< const world::IEntityFactory > entityFactory = m_entityBuilder->getFactory(entityData);
	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for \"" << type_name(entityData) << L"\"" << Endl;
		return 0;
	}

	Ref< world::Entity > entity;
	{
		T_ANONYMOUS_VAR(Save< Ref< EntityAdapter > >)(m_currentAdapter, entityAdapter);

		// See if we can find an entity in the leaf hash.
		if (!cache.leafEntities.empty())
		{
			uint32_t hash = DeepHash(entityData).get();
			RefArray< world::Entity >& entities = cache.leafEntities[hash];
			if (!entities.empty())
			{
				entity = entities.front();
				entities.pop_front();
			}
		}

		// If no leaf entity then we need to re-create the entity.
		if (!entity)
		{
			if (m_buildTimeStack.empty())
				m_timer.start();

			m_buildTimeStack.push_back(m_timer.getElapsedTime());

			entity = entityFactory->createEntity(this, *entityData);

			double duration = m_timer.getElapsedTime() - m_buildTimeStack.back();
			m_buildTimes[&type_of(entityData)].first++;
			m_buildTimes[&type_of(entityData)].second += duration;
			m_buildTimeStack.pop_back();

			// Remove duration from parent build steps; not inclusive times.
			for (std::vector< double >::iterator i = m_buildTimeStack.begin(); i != m_buildTimeStack.end(); ++i)
				*i += duration;
		}

		// If still no entity then we create a null placeholder.
		if (!entity)
		{
			log::warning << L"Unable to create entity from \"" << type_name(entityData) << L"\"; using null entity as placeholder" << Endl;
			entity = new world::NullEntity(entityData->getTransform());
		}
	}

	entityAdapter->setEntityData(const_cast< world::EntityData* >(entityData));
	entityAdapter->setEntity(entity);

	if (!entityAdapter->getEntityEditor())
	{
		Ref< IEntityEditor > entityEditor = createEntityEditor(m_context, m_entityEditorFactories, entityAdapter);
		if (!entityEditor)
		{
			log::error << L"Unable to create entity editor from \"" << type_name(entityData) << L"\"" << Endl;
			return 0;
		}

		entityAdapter->setEntityEditor(entityEditor);
	}

	++m_adapterCount;
	return entity;
}

Ref< world::IEntityEvent > EntityAdapterBuilder::create(const world::IEntityEventData* entityEventData) const
{
	return m_entityBuilder->create(entityEventData);
}

const world::IEntityBuilder* EntityAdapterBuilder::getCompositeEntityBuilder() const
{
	return m_entityBuilder->getCompositeEntityBuilder();
}

EntityAdapter* EntityAdapterBuilder::getRootAdapter() const
{
	return m_rootAdapter;
}

uint32_t EntityAdapterBuilder::getAdapterCount() const
{
	return m_adapterCount;
}

	}
}

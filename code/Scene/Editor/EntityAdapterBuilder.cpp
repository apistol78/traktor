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

		// Destroy existing entity; will be re-created.
		Ref< world::Entity > entity = entityAdapter->getEntity();
		if (entity)
		{
			entity->destroy();
			entityAdapter->setEntity(0);
		}

		// Unlink adapter from parent.
		EntityAdapter* parent = entityAdapter->getParent();
		if (parent)
			parent->unlink(entityAdapter);

		// Insert into map from instance guid to adapters.
		if (entityAdapter->getEntityData())
		{
			const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());
			m_cachedAdapters[&entityDataType].push_back(entityAdapter);
		}

		// Release entity data reference.
		entityAdapter->setEntityData(0);
	}

	m_adapterCount = 0;

	T_ASSERT (!m_rootAdapter);
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

	// Get adapter; reuse adapters containing same type of entity.
	RefArray< EntityAdapter >& cachedAdapters = m_cachedAdapters[&type_of(entityData)];
	if (!cachedAdapters.empty())
	{
		entityAdapter = cachedAdapters.front();
		cachedAdapters.pop_front();
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
		if (!(entity = entityFactory->createEntity(this, *entityData)))
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

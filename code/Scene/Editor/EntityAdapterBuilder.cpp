#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/DeepHash.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/IEntityFactory.h"
#include "World/Entity/IEntitySchema.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

Ref< IEntityEditor > createEntityEditor(
	SceneEditorContext* context,
	const RefArray< IEntityEditorFactory >& entityEditorFactories,
	EntityAdapter* entityAdapter
)
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< IEntityEditorFactory > entityEditorFactory;

	const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());

	for (RefArray< IEntityEditorFactory >::const_iterator i = entityEditorFactories.begin(); i != entityEditorFactories.end(); ++i)
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

EntityAdapterBuilder::EntityAdapterBuilder(SceneEditorContext* context, const RefArray< IEntityEditorFactory >& entityEditorFactories)
:	m_context(context)
,	m_entityEditorFactories(entityEditorFactories)
{
}

void EntityAdapterBuilder::addFactory(world::IEntityFactory* entityFactory)
{
	m_entityFactories.push_back(entityFactory);
}

void EntityAdapterBuilder::removeFactory(world::IEntityFactory* entityFactory)
{
	T_BREAKPOINT;
}

void EntityAdapterBuilder::begin(world::IEntitySchema* entitySchema)
{
	m_entitySchema = entitySchema;

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters, SceneEditorContext::GfDescendants | SceneEditorContext::GfExternals);

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
		m_cachedAdapters[entityAdapter->getEntityData()].push_back(entityAdapter);
	}

	T_ASSERT (!m_rootAdapter);
}

Ref< world::Entity > EntityAdapterBuilder::create(const world::EntityData* entityData)
{
	Ref< EntityAdapter > entityAdapter;

	if (!entityData)
		return 0;

	// Get adapter; reuse adapters containing same type of entity.
	RefArray< EntityAdapter >& cachedAdapters = m_cachedAdapters[entityData];
	if (!cachedAdapters.empty())
	{
		entityAdapter = cachedAdapters.front();
		cachedAdapters.pop_front();
	}
	else
	{
		entityAdapter = new EntityAdapter(const_cast< world::EntityData* >(entityData));
	}

	if (m_currentAdapter)
		m_currentAdapter->link(entityAdapter);
	else
	{
		T_ASSERT (!m_rootAdapter);
		m_rootAdapter = entityAdapter;
	}

	// Find entity factory.
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< world::IEntityFactory > entityFactory;

	for (RefArray< world::IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
	{
		const TypeInfoSet& typeSet = (*i)->getEntityTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
		{
			if (is_type_of(**j, type_of(entityData)))
			{
				uint32_t classDifference = type_difference(**j, type_of(entityData));
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					entityFactory = *i;
				}
			}
		}
	}

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
			log::error << L"Unable to create entity from \"" << type_name(entityData) << L"\"" << Endl;
			return 0;
		}
	}

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

	m_entities[entityData] = entity;
	
	if (m_entitySchema)
		m_entitySchema->insertEntity(0, entityData->getName(), entity);

	return entity;
}

Ref< world::Entity > EntityAdapterBuilder::get(const world::EntityData* entityData) const
{
	std::map< const world::EntityData*, Ref< world::Entity > >::const_iterator i = m_entities.find(entityData);
	return (i != m_entities.end()) ? i->second : 0;
}

void EntityAdapterBuilder::end()
{
	T_ASSERT (m_currentAdapter == 0);
	m_cachedAdapters.clear();
}

Ref< EntityAdapter > EntityAdapterBuilder::getRootAdapter() const
{
	return m_rootAdapter;
}

	}
}

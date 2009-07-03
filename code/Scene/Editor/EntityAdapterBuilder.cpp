#include <limits>
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "World/Entity/IEntityFactory.h"
#include "World/Entity/IEntityManager.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Misc/Save.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterBuilder", EntityAdapterBuilder, world::IEntityBuilder)

EntityAdapterBuilder::EntityAdapterBuilder(SceneEditorContext* context)
:	m_context(context)
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

void EntityAdapterBuilder::begin(world::IEntityManager* entityManager)
{
	m_entityManager = entityManager;

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

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

		// Unlink adapter from hierarchy.
		entityAdapter->unlink();

		// Insert into map from instance to adapter.
		m_instances.insert(std::make_pair(entityAdapter->getInstance(), entityAdapter));
	}

	T_ASSERT (!m_rootAdapter);
}

world::Entity* EntityAdapterBuilder::create(const std::wstring& name, const world::EntityData* entityData)
{
	if (!entityData)
		return 0;

	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< world::IEntityFactory > entityFactory;

	for (RefArray< world::IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
	{
		const TypeSet& typeSet = (*i)->getEntityTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
		{
			if (is_type_of(**j, entityData->getType()))
			{
				uint32_t classDifference = type_difference(**j, entityData->getType());
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

	Ref< world::Entity > entity = entityFactory->createEntity(this, name, *entityData);
	if (!entity)
	{
		log::error << L"Unable to create entity from \"" << type_name(entityData) << L"\"" << Endl;
		return 0;
	}

	// Add this entity to the manager.
	if (m_entityManager)
		m_entityManager->insertEntity(entity);

	return entity;
}

world::Entity* EntityAdapterBuilder::build(const world::EntityInstance* instance)
{
	if (!instance)
		return 0;

	Ref< EntityAdapter > entityAdapter;
	Ref< world::Entity > entity;

	std::map< const world::EntityInstance*, Ref< EntityAdapter > >::iterator i = m_instances.find(instance);
	if (i != m_instances.end())
		entityAdapter = i->second;
	else
		entityAdapter = gc_new< EntityAdapter >(const_cast< world::EntityInstance* >(instance));

	if (m_currentAdapter)
		m_currentAdapter->addChild(entityAdapter, false);
	else
	{
		T_ASSERT (!m_rootAdapter);
		m_rootAdapter = entityAdapter;
	}

	{
		Save< Ref< EntityAdapter > > scope(m_currentAdapter, entityAdapter);
		entity = create(instance->getName(), instance->getEntityData());
	}

	entityAdapter->setEntity(entity);

	return entity;
}

void EntityAdapterBuilder::end()
{
	T_ASSERT (m_currentAdapter == 0);
	m_instances.clear();
}

EntityAdapter* EntityAdapterBuilder::getRootAdapter() const
{
	return m_rootAdapter;
}

	}
}

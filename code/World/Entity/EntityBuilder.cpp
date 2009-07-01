#include <limits>
#include <algorithm>
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "World/Entity/IEntityManager.h"
#include "World/Entity/IEntityFactory.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, Object)

void EntityBuilder::addFactory(IEntityFactory* entityFactory)
{
	m_entityFactories.push_back(entityFactory);
}

void EntityBuilder::removeFactory(IEntityFactory* entityFactory)
{
	RefArray< IEntityFactory >::iterator i = std::find(m_entityFactories.begin(), m_entityFactories.end(), entityFactory);
	if (i != m_entityFactories.end())
		m_entityFactories.erase(i);
}

void EntityBuilder::begin(IEntityManager* entityManager)
{
	m_entityManager = entityManager;
}

Entity* EntityBuilder::create(const std::wstring& name, const EntityData* entityData)
{
	if (!entityData)
		return 0;

	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< IEntityFactory > entityFactory;

	for (RefArray< IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
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

	Ref< Entity > entity = entityFactory->createEntity(this, name, *entityData);
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

Entity* EntityBuilder::build(const EntityInstance* instance)
{
	if (!instance)
		return 0;

	// If we've already created this instance then we return same entity.
	std::map< const EntityInstance*, Ref< Entity > >::iterator i = m_instances.find(instance);
	if (i != m_instances.end())
		return i->second;

	// Create entity from entity data.
	Ref< const EntityData > entityData = instance->getEntityData();
	Ref< Entity > entity = create(instance->getName(), entityData);
	if (!entity)
		return 0;

	// Resolve references to this entity instance.
	const RefArray< EntityInstance >& references = instance->getReferences();
	for (RefArray< EntityInstance >::const_iterator i = references.begin(); i != references.end(); ++i)
	{
		Ref< Entity > reference = build(*i);
		if (reference)
		{
			entity->addReference(reference);
			reference->addReferee(entity);
		}
		else
			log::warning << L"Unable to get reference instance" << Endl;
	}

	// Save to instances cache.
	m_instances.insert(std::make_pair(instance, entity));
	return entity;
}

void EntityBuilder::end()
{
	m_instances.clear();
}

	}
}

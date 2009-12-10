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

EntityBuilder::EntityBuilder()
:	m_inbuild(false)
{
}

void EntityBuilder::addFactory(IEntityFactory* entityFactory)
{
	T_FATAL_ASSERT_M (!m_inbuild, L"Should not add entity factory when building");
	m_entityFactories.push_back(entityFactory);
}

void EntityBuilder::removeFactory(IEntityFactory* entityFactory)
{
	T_FATAL_ASSERT_M (!m_inbuild, L"Should not remove entity factory when building");
	RefArray< IEntityFactory >::iterator i = std::find(m_entityFactories.begin(), m_entityFactories.end(), entityFactory);
	if (i != m_entityFactories.end())
		m_entityFactories.erase(i);
}

void EntityBuilder::begin(IEntityManager* entityManager)
{
	T_FATAL_ASSERT_M (!m_inbuild, L"EntityBuilder already begun");
	m_entityManager = entityManager;
	m_inbuild = true;
}

Ref< Entity > EntityBuilder::create(const std::wstring& name, const EntityData* entityData, const Object* instanceData)
{
	T_FATAL_ASSERT_M (m_inbuild, L"EntityBuilder not begun");

	Ref< const EntityData > entityDataRef(entityData);
	if (!entityDataRef)
		return 0;

	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< IEntityFactory > entityFactory;

	for (RefArray< IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
	{
		const TypeInfoSet& typeSet = (*i)->getEntityTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
		{
			if (is_type_of(**j, type_of(entityDataRef)))
			{
				uint32_t classDifference = type_difference(**j, type_of(entityDataRef));
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
		log::error << L"Unable to find entity factory for \"" << type_name(entityDataRef) << L"\"" << Endl;
		return 0;
	}

	Ref< Entity > entity = entityFactory->createEntity(this, name, *entityDataRef, instanceData);
	if (!entity)
	{
		log::error << L"Unable to create entity from \"" << type_name(entityDataRef) << L"\"" << Endl;
		return 0;
	}

	// Add this entity to the manager.
	if (m_entityManager)
		m_entityManager->insertEntity(name, entity);

	return entity;
}

Ref< Entity > EntityBuilder::build(const EntityInstance* instance)
{
	T_FATAL_ASSERT_M (m_inbuild, L"EntityBuilder not begun");

	Ref< const EntityInstance > instanceRef(instance);
	if (!instanceRef)
		return 0;

	// If we've already created this instance then we return same entity.
	std::map< Ref< const EntityInstance >, Ref< Entity > >::iterator i = m_instances.find(instanceRef);
	if (i != m_instances.end())
		return i->second;

	// Create entity from entity data.
	Ref< const EntityData > entityData = instanceRef->getEntityData();
	Ref< Entity > entity = create(instanceRef->getName(), entityData, instanceRef->getInstanceData());
	if (!entity)
		return 0;

	// Save to instances cache.
	m_instances.insert(std::make_pair(
		instanceRef,
		entity
	));

	// Resolve references to this entity instance.
	const std::vector< EntityInstance* >& references = instanceRef->getReferences();
	for (std::vector< EntityInstance* >::const_iterator i = references.begin(); i != references.end(); ++i)
	{
		Ref< Entity > reference = build(*i);
		if (reference)
		{
			entity->addReference((*i)->getName(), reference);
			reference->addReferee(instanceRef->getName(), entity);
		}
		else
			log::warning << L"Unable to get reference instance" << Endl;
	}

	return entity;
}

void EntityBuilder::end()
{
	T_FATAL_ASSERT_M (m_inbuild, L"EntityBuilder not begun");
	m_instances.clear();
	m_inbuild = false;
}

	}
}

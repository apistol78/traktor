#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/IEntityManager.h"
#include "World/Entity/IEntityFactory.h"

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

Ref< Entity > EntityBuilder::create(const EntityData* entityData)
{
	T_FATAL_ASSERT_M (m_inbuild, L"EntityBuilder not begun");

	if (!entityData)
		return 0;

	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< IEntityFactory > entityFactory;

	for (RefArray< IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
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

	Ref< Entity > entity = entityFactory->createEntity(this, *entityData);
	if (!entity)
	{
		log::error << L"Unable to create entity from \"" << type_name(entityData) << L"\"" << Endl;
		return 0;
	}

	m_entities[entityData] = entity;

	if (m_entityManager)
		m_entityManager->insertEntity(entityData->getName(), entity);

	return entity;
}

Ref< Entity > EntityBuilder::get(const EntityData* entityData) const
{
	std::map< const EntityData*, Ref< Entity > >::const_iterator i = m_entities.find(entityData);
	return (i != m_entities.end()) ? i->second : 0;
}

void EntityBuilder::end()
{
	T_FATAL_ASSERT_M (m_inbuild, L"EntityBuilder not begun");
	m_entities.clear();
	m_inbuild = false;
}

	}
}

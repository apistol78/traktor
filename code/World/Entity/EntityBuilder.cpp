#include <algorithm>
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/EntityFactory.h"
#include "World/Entity/EntityData.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, Object)

void EntityBuilder::addFactory(EntityFactory* entityFactory)
{
	m_entityFactories.push_back(entityFactory);
}

void EntityBuilder::removeFactory(EntityFactory* entityFactory)
{
	RefArray< EntityFactory >::iterator i = std::find(m_entityFactories.begin(), m_entityFactories.end(), entityFactory);
	if (i != m_entityFactories.end())
		m_entityFactories.erase(i);
}

void EntityBuilder::setEntityManager(EntityManager* entityManager)
{
	m_entityManager = entityManager;
}

EntityManager* EntityBuilder::getEntityManager() const
{
	return m_entityManager;
}

Entity* EntityBuilder::build(const EntityData* entityData)
{
	if (!entityData)
		return 0;

	Ref< Entity > entity;

	for (RefArray< EntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && !entity; ++i)
	{
		const TypeSet& typeSet = (*i)->getEntityTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end() && !entity; ++j)
		{
			if (is_type_of(*(*j), entityData->getType()))
				entity = (*i)->createEntity(this, *entityData);
		}
	}

	if (entity)
	{
		if (m_entityManager)
			m_entityManager->insertEntity(entity);
	}
	else
		log::error << L"Unable to create entity from \"" << type_name(entityData) << L"\"" << Endl;

	return entity;
}

	}
}

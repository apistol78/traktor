#include <limits>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "World/Entity.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/IEntityEventData.h"
#include "World/IEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, IEntityBuilder)

void EntityBuilder::addFactory(const IEntityFactory* entityFactory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_entityFactories.push_back(entityFactory);
	m_resolvedFactoryCache.clear();
}

void EntityBuilder::removeFactory(const IEntityFactory* entityFactory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	RefArray< const IEntityFactory >::iterator i = std::find(m_entityFactories.begin(), m_entityFactories.end(), entityFactory);
	if (i != m_entityFactories.end())
	{
		m_entityFactories.erase(i);
		m_resolvedFactoryCache.clear();
	}
}

const IEntityFactory* EntityBuilder::getFactory(const EntityData* entityData) const
{
	if (!entityData)
		return 0;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const TypeInfo& entityDataType = type_of(entityData);
	const IEntityFactory* entityFactory = 0;

	std::map< const TypeInfo*, const IEntityFactory* >::const_iterator i = m_resolvedFactoryCache.find(&entityDataType);
	if (i != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = i->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
		{
			const TypeInfoSet& typeSet = (*i)->getEntityTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityDataType))
				{
					uint32_t classDifference = type_difference(**j, entityDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *i;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&entityDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for \"" << entityData->getName() << L"\" of " << type_name(entityData) << Endl;
		return 0;
	}

	return entityFactory;
}

const IEntityFactory* EntityBuilder::getFactory(const IEntityEventData* entityEventData) const
{
	if (!entityEventData)
		return 0;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const TypeInfo& entityEventDataType = type_of(entityEventData);
	const IEntityFactory* entityFactory = 0;

	std::map< const TypeInfo*, const IEntityFactory* >::const_iterator i = m_resolvedFactoryCache.find(&entityEventDataType);
	if (i != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = i->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
		{
			const TypeInfoSet& typeSet = (*i)->getEntityEventTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityEventDataType))
				{
					uint32_t classDifference = type_difference(**j, entityEventDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *i;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&entityEventDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for event of " << type_name(entityEventData) << Endl;
		return 0;
	}

	return entityFactory;
}

Ref< Entity > EntityBuilder::create(const EntityData* entityData) const
{
	Ref< const IEntityFactory > entityFactory = getFactory(entityData);
	return entityFactory ? entityFactory->createEntity(this, *entityData) : 0;
}

Ref< IEntityEvent > EntityBuilder::create(const IEntityEventData* entityEventData) const
{
	Ref< const IEntityFactory > entityFactory = getFactory(entityEventData);
	return entityFactory ? entityFactory->createEntityEvent(this, *entityEventData) : 0;
}

const IEntityBuilder* EntityBuilder::getCompositeEntityBuilder() const
{
	return this;
}

	}
}

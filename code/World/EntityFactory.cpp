/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "World/EntityData.h"
#include "World/EntityFactory.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"
#include "World/IWorldComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityFactory", EntityFactory, IEntityFactory)

void EntityFactory::addFactory(const IEntityFactory* entityFactory)
{
	m_factories.push_back(entityFactory);
	m_resolvedFactoryCache.clear();
}

void EntityFactory::removeFactory(const IEntityFactory* entityFactory)
{
	if (m_factories.remove(entityFactory))
		m_resolvedFactoryCache.clear();
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	for (auto factory : m_factories)
	{
		const TypeInfoSet factoryTypeSet = factory->getEntityTypes();
		typeSet.insert(factoryTypeSet.begin(), factoryTypeSet.end());
	}
	return typeSet;
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	TypeInfoSet typeSet;
	for (auto factory : m_factories)
	{
		const TypeInfoSet factoryTypeSet = factory->getEntityEventTypes();
		typeSet.insert(factoryTypeSet.begin(), factoryTypeSet.end());
	}
	return typeSet;
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	for (auto factory : m_factories)
	{
		const TypeInfoSet factoryTypeSet = factory->getEntityComponentTypes();
		typeSet.insert(factoryTypeSet.begin(), factoryTypeSet.end());
	}
	return typeSet;
}

const TypeInfoSet EntityFactory::getWorldComponentTypes() const
{
	TypeInfoSet typeSet;
	for (auto factory : m_factories)
	{
		const TypeInfoSet factoryTypeSet = factory->getWorldComponentTypes();
		typeSet.insert(factoryTypeSet.begin(), factoryTypeSet.end());
	}
	return typeSet;
}

Ref< Entity > EntityFactory::createEntity(const IEntityBuilder* builder, const EntityData& entityData) const
{
	const IEntityFactory* factory = getFactory(&entityData);
	return factory != nullptr ? factory->createEntity(builder, entityData) : nullptr;
}

Ref< IEntityEvent > EntityFactory::createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const
{
	const IEntityFactory* factory = getFactory(&entityEventData);
	return factory != nullptr ? factory->createEntityEvent(builder, entityEventData) : nullptr;
}

Ref< IEntityComponent > EntityFactory::createEntityComponent(const IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const
{
	const IEntityFactory* factory = getFactory(&entityComponentData);
	return factory != nullptr ? factory->createEntityComponent(builder, entityComponentData) : nullptr;
}

Ref< IWorldComponent > EntityFactory::createWorldComponent(const IEntityBuilder* builder, const IWorldComponentData& worldComponentData) const
{
	const IEntityFactory* factory = getFactory(&worldComponentData);
	return factory != nullptr ? factory->createWorldComponent(builder, worldComponentData) : nullptr;
}

const IEntityFactory* EntityFactory::getFactory(const EntityData* entityData) const
{
	if (!entityData)
		return nullptr;

	const TypeInfo& entityDataType = type_of(entityData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&entityDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_factories.begin(); it != m_factories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getEntityTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityDataType))
				{
					const uint32_t classDifference = type_difference(**j, entityDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
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
		log::error << L"Unable to find entity factory for \"" << entityData->getName() << L"\" of \"" << type_name(entityData) << L"\"." << Endl;
		return nullptr;
	}

	return entityFactory;
}

const IEntityFactory* EntityFactory::getFactory(const IEntityEventData* entityEventData) const
{
	if (!entityEventData)
		return nullptr;

	const TypeInfo& entityEventDataType = type_of(entityEventData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&entityEventDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_factories.begin(); it != m_factories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getEntityEventTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityEventDataType))
				{
					const uint32_t classDifference = type_difference(**j, entityEventDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
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
		log::error << L"Unable to find entity factory for event of \"" << type_name(entityEventData) << L"\"." << Endl;
		return nullptr;
	}

	return entityFactory;
}

const IEntityFactory* EntityFactory::getFactory(const IEntityComponentData* entityComponentData) const
{
	if (!entityComponentData)
		return nullptr;

	const TypeInfo& entityComponentDataType = type_of(entityComponentData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&entityComponentDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity component has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity component from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_factories.begin(); it != m_factories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getEntityComponentTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityComponentDataType))
				{
					const uint32_t classDifference = type_difference(**j, entityComponentDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&entityComponentDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for component of \"" << type_name(entityComponentData) << L"\"." << Endl;
		return nullptr;
	}

	return entityFactory;
}

const IEntityFactory* EntityFactory::getFactory(const IWorldComponentData* worldComponentData) const
{
	if (!worldComponentData)
		return nullptr;

	const TypeInfo& worldComponentDataType = type_of(worldComponentData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&worldComponentDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity component has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity component from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_factories.begin(); it != m_factories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getWorldComponentTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, worldComponentDataType))
				{
					const uint32_t classDifference = type_difference(**j, worldComponentDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&worldComponentDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for component of \"" << type_name(worldComponentData) << L"\"." << Endl;
		return nullptr;
	}

	return entityFactory;
}

}

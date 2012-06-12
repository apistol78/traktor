#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/IEntitySchema.h"
#include "World/Entity/IEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, IEntityBuilder)

EntityBuilder::EntityBuilder()
:	m_inbuild(false)
{
}

void EntityBuilder::addFactory(IEntityFactory* entityFactory)
{
	T_FATAL_ASSERT_M (!m_inbuild, L"Should not add entity factory when building");
	m_entityFactories.push_back(entityFactory);
	m_resolvedFactoryCache.clear();
}

void EntityBuilder::removeFactory(IEntityFactory* entityFactory)
{
	T_FATAL_ASSERT_M (!m_inbuild, L"Should not remove entity factory when building");
	RefArray< IEntityFactory >::iterator i = std::find(m_entityFactories.begin(), m_entityFactories.end(), entityFactory);
	if (i != m_entityFactories.end())
	{
		m_entityFactories.erase(i);
		m_resolvedFactoryCache.clear();
	}
}

void EntityBuilder::begin(IEntitySchema* entitySchema)
{
	T_FATAL_ASSERT_M (!m_inbuild, L"EntityBuilder already begun");
	m_entitySchema = entitySchema;
	m_inbuild = true;
}

Ref< Entity > EntityBuilder::create(const EntityData* entityData)
{
	T_FATAL_ASSERT_M (m_inbuild, L"EntityBuilder not begun");

	if (!entityData)
		return 0;

	const TypeInfo& entityDataType = type_of(entityData);
	IEntityFactory* entityFactory = 0;

	std::map< const TypeInfo*, IEntityFactory* >::const_iterator i = m_resolvedFactoryCache.find(&entityDataType);
	if (i != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = i->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
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

		m_resolvedFactoryCache.insert(std::make_pair(&entityDataType, entityFactory));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for \"" << type_name(entityData) << L"\"" << Endl;
		return 0;
	}

	m_entityScope.push(scope_t());

	Ref< Entity > entity = entityFactory->createEntity(this, *entityData);
	if (!entity)
	{
		log::error << L"Unable to create entity from \"" << type_name(entityData) << L"\"" << Endl;
		m_entityScope.pop();
		return 0;
	}

	if (m_entitySchema)
	{
		const scope_t& scope = m_entityScope.top();
		for (scope_t::const_iterator i = scope.begin(); i != scope.end(); ++i)
		{
			if (i->second != entity)
				m_entitySchema->insertEntity(entity, i->first, i->second);
		}
	}

	m_entityScope.pop();

	if (m_entitySchema)
	{
		if (!m_entityScope.empty())
		{
			scope_t& scope = m_entityScope.top();
			scope.push_back(std::make_pair(entityData->getName(), entity));
		}
		else
		{
			m_entitySchema->insertEntity(0, entityData->getName(), entity);
		}
	}

	m_entities[entityData] = entity;

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
	m_entitySchema = 0;
	m_inbuild = false;
}

	}
}

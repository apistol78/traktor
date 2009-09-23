#include <algorithm>
#include "World/Entity/EntityManager.h"
#include "World/Entity/Entity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityManager", EntityManager, IEntityManager)

void EntityManager::addEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Cannot add null entity");
	T_ASSERT_M (std::find(m_entities.begin(), m_entities.end(), entity) == m_entities.end(), L"Entity already added");
	
	const Type& entityType = entity->getType();
	if (m_typeRanges.find(&entityType) != m_typeRanges.end())
	{
		Range& range = m_typeRanges[&entityType];
		m_entities.insert(
			m_entities.begin() + range.start + range.count,
			entity
		);

		for (std::map< const Type*, Range >::iterator i = m_typeRanges.begin(); i != m_typeRanges.end(); ++i)
		{
			if (i->second.start >= (range.start + range.count))
				++i->second.start;
		}

		++range.count;
	}
	else
	{
		Range range = { uint32_t(m_entities.size()), 1 };
		m_typeRanges.insert(std::make_pair(&entityType, range));
		m_entities.push_back(entity);
	}
}

void EntityManager::insertEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Cannot insert null entity");

	const Type& entityType = entity->getType();
	if (m_typeRanges.find(&entityType) != m_typeRanges.end())
	{
		const Range& range = m_typeRanges[&entityType];
		for (uint32_t i = 0; i < range.count; ++i)
		{
			if (m_entities[i + range.start] == entity)
				return;
		}
	}

	addEntity(entity);
}

void EntityManager::removeEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Cannot remove null entity");

	const Type& entityType = entity->getType();
	if (m_typeRanges.find(&entityType) != m_typeRanges.end())
	{
		Range& range = m_typeRanges[&entityType];
		if (range.count > 0)
		{
			RefArray< Entity >::iterator i = std::find(m_entities.begin() + range.start, m_entities.begin() + range.start + range.count, entity);
			m_entities.erase(i);

			for (std::map< const Type*, Range >::iterator i = m_typeRanges.begin(); i != m_typeRanges.end(); ++i)
			{
				if (i->second.start >= (range.start + range.count))
					--i->second.start;
			}

			--range.count;
		}
	}
}

uint32_t EntityManager::getEntitiesOf(const Type& entityType, RefArray< Entity >& outEntities) const
{
	RefArray< Entity >::lock_wr_type(outEntities.lock());
	for (std::map< const Type*, Range >::const_iterator i = m_typeRanges.begin(); i != m_typeRanges.end(); ++i)
	{
		if (is_type_of(entityType, *i->first))
		{
			for (uint32_t j = 0; j < i->second.count; ++j)
				outEntities.push_back_unsafe(m_entities[i->second.start + j]);
		}
	}
	return uint32_t(outEntities.size());
}

uint32_t EntityManager::getEntityCount(const Type& entityType) const
{
	std::map< const Type*, Range >::const_iterator i = m_typeRanges.find(&entityType);
	return i != m_typeRanges.end() ? i->second.count : 0UL;
}

Entity* EntityManager::getEntity(const Type& entityType, uint32_t index) const
{
	std::map< const Type*, Range >::const_iterator i = m_typeRanges.find(&entityType);
	if (i == m_typeRanges.end() || index >= i->second.count)
		return 0;

	return m_entities[i->second.start + index];
}

const RefArray< Entity >& EntityManager::getEntities() const
{
	return m_entities;
}

	}
}

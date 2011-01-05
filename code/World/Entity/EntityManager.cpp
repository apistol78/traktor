#include <algorithm>
#include "World/Entity/EntityManager.h"
#include "World/Entity/Entity.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

template < typename pair_t >
struct pair_second_pred
{
	typename pair_t::second_type m_value;

	pair_second_pred(const typename pair_t::second_type& value)
		:	m_value(value)
	{
	}

	bool operator () (const pair_t& _lh) const
	{
		return _lh.second == m_value;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityManager", EntityManager, IEntityManager)

void EntityManager::addEntity(const std::wstring& name, Entity* entity)
{
	T_ASSERT_M (entity, L"Cannot add null entity");
	T_ASSERT_M (std::find_if(m_entities.begin(), m_entities.end(), pair_second_pred< named_entity_t >(entity)) == m_entities.end(), L"Entity already added");
	
	const TypeInfo& entityType = type_of(entity);
	if (m_typeRanges.find(&entityType) != m_typeRanges.end())
	{
		Range& range = m_typeRanges[&entityType];
		m_entities.insert(
			m_entities.begin() + range.start + range.count,
			std::make_pair(name, entity)
		);

		for (std::map< const TypeInfo*, Range >::iterator i = m_typeRanges.begin(); i != m_typeRanges.end(); ++i)
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
		m_entities.push_back(std::make_pair(name, entity));
	}
}

void EntityManager::insertEntity(const std::wstring& name, Entity* entity)
{
	T_ASSERT_M (entity, L"Cannot insert null entity");

	const TypeInfo& entityType = type_of(entity);
	if (m_typeRanges.find(&entityType) != m_typeRanges.end())
	{
		const Range& range = m_typeRanges[&entityType];
		for (uint32_t i = 0; i < range.count; ++i)
		{
			if (m_entities[i + range.start].second == entity)
				return;
		}
	}

	addEntity(name, entity);
}

void EntityManager::removeEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Cannot remove null entity");

	const TypeInfo& entityType = type_of(entity);
	if (m_typeRanges.find(&entityType) != m_typeRanges.end())
	{
		Range& range = m_typeRanges[&entityType];
		if (range.count > 0)
		{
			named_entity_vector_t::iterator i = std::find_if(
				m_entities.begin() + range.start,
				m_entities.begin() + range.start + range.count,
				pair_second_pred< named_entity_t >(entity)
			);
			m_entities.erase(i);

			for (std::map< const TypeInfo*, Range >::iterator i = m_typeRanges.begin(); i != m_typeRanges.end(); ++i)
			{
				if (i->second.start >= (range.start + range.count))
					--i->second.start;
			}

			--range.count;
		}
	}
}

Entity* EntityManager::getEntity(const std::wstring& name) const
{
	for (named_entity_vector_t::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (i->first == name)
			return i->second;
	}
	return 0;
}

uint32_t EntityManager::getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const
{
	for (named_entity_vector_t::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (i->first == name)
			outEntities.push_back(i->second);
	}
	return outEntities.size();
}

uint32_t EntityManager::getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	for (std::map< const TypeInfo*, Range >::const_iterator i = m_typeRanges.begin(); i != m_typeRanges.end(); ++i)
	{
		if (is_type_of(entityType, *i->first))
		{
			for (uint32_t j = 0; j < i->second.count; ++j)
				outEntities.push_back(m_entities[i->second.start + j].second);
		}
	}
	return uint32_t(outEntities.size());
}

uint32_t EntityManager::getEntityCount(const TypeInfo& entityType) const
{
	std::map< const TypeInfo*, Range >::const_iterator i = m_typeRanges.find(&entityType);
	return i != m_typeRanges.end() ? i->second.count : 0UL;
}

Entity* EntityManager::getEntity(const TypeInfo& entityType, uint32_t index) const
{
	std::map< const TypeInfo*, Range >::const_iterator i = m_typeRanges.find(&entityType);
	if (i == m_typeRanges.end() || index >= i->second.count)
		return 0;

	return m_entities[i->second.start + index].second;
}

Entity* EntityManager::getEntity(const TypeInfo& entityType, const std::wstring& name) const
{
	std::map< const TypeInfo*, Range >::const_iterator i = m_typeRanges.find(&entityType);
	if (i == m_typeRanges.end())
		return 0;

	for (uint32_t j = 0; j < i->second.count; ++j)
	{
		if (name == m_entities[i->second.start + j].first)
			return m_entities[i->second.start + j].second;
	}

	return 0;
}

	}
}

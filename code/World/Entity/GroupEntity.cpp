#include <algorithm>
#include "World/Entity/GroupEntity.h"
#include "Core/Misc/Save.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntity", GroupEntity, Entity)

GroupEntity::GroupEntity()
:	m_update(false)
{
}

GroupEntity::~GroupEntity()
{
	destroy();
}

void GroupEntity::destroy()
{
	T_ASSERT_M (!m_update, L"Cannot destroy group while in update");
	T_ASSERT (m_remove.empty());
	for (RefArray< Entity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_entities.resize(0);
}

void GroupEntity::addEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	T_ASSERT_M (!m_update, L"Not allowed to add entity during update");
	m_entities.push_back(entity);
}

void GroupEntity::removeEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	if (m_update)
	{
		// Add as deferred remove; we cannot remove while update
		// is iterating entity array.
		m_remove.push_back(entity);
	}
	else
	{
		RefArray< Entity >::iterator i = std::find(m_entities.begin(), m_entities.end(), entity);
		m_entities.erase(i);
	}
}

void GroupEntity::removeAllEntities()
{
	T_ASSERT_M (!m_update, L"Cannot remove all entities while in update; not implemented");
	m_entities.resize(0);
}

const RefArray< Entity >& GroupEntity::getEntities() const
{
	return m_entities;
}
	
int GroupEntity::getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (is_type_of(entityType, type_of(*i)))
			outEntities.push_back(*i);
	}
	return int(outEntities.size());
}

Ref< Entity > GroupEntity::getFirstEntityOf(const TypeInfo& entityType) const
{
	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (is_type_of(entityType, type_of(*i)))
			return *i;
	}
	return 0;
}

int GroupEntity::getEntitiesOfRecursive(const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	getEntitiesOf(entityType, outEntities);

	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (GroupEntity* childGroup = dynamic_type_cast< GroupEntity* >(*i))
			childGroup->getEntitiesOfRecursive(entityType, outEntities);
	}

	return int(outEntities.size());
}

Ref< Entity > GroupEntity::getFirstEntityOfRecursive(const TypeInfo& entityType) const
{
	Ref< Entity > entity = getFirstEntityOf(entityType);
	if (!entity)
	{
		for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
		{
			if (GroupEntity* childGroup = dynamic_type_cast< GroupEntity* >(*i))
			{
				entity = childGroup->getFirstEntityOfRecursive(entityType);
				if (entity)
					break;
			}
		}
	}
	return entity;
}

void GroupEntity::update(const EntityUpdate* update)
{
	// Update child entities; set flag to indicate we're
	// updating 'em.
	{
		Save< bool > scope(m_update, true);
		for (RefArray< Entity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
			(*i)->update(update);
	}
	// Remove deferred removed entities.
	if (!m_remove.empty())
	{
		for (RefArray< Entity >::iterator i = m_remove.begin(); i != m_remove.end(); ++i)
			removeEntity(*i);
		m_remove.resize(0);
	}
}

	}
}

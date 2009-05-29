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

GroupEntity::GroupEntity(const std::wstring& name)
:	m_name(name)
,	m_update(false)
{
}

GroupEntity::~GroupEntity()
{
	destroy();
}

void GroupEntity::destroy()
{
	T_ASSERT (m_remove.empty());
	for (RefArray< Entity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_entities.resize(0);
}

const std::wstring& GroupEntity::getName() const
{
	return m_name;
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
	
int GroupEntity::getEntitiesOf(const Type& entityType, RefArray< Entity >& outEntities) const
{
	outEntities.lock();
	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (is_type_of(entityType, (*i)->getType()))
			outEntities.push_back_unsafe(*i);
	}
	outEntities.unlock();
	return int(outEntities.size());
}

Entity* GroupEntity::getFirstEntityOf(const Type& entityType) const
{
	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (is_type_of(entityType, (*i)->getType()))
			return *i;
	}
	return 0;
}

int GroupEntity::getEntitiesOfRecursive(const Type& entityType, RefArray< Entity >& outEntities) const
{
	getEntitiesOf(entityType, outEntities);

	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (GroupEntity* childGroup = dynamic_type_cast< GroupEntity* >(*i))
			childGroup->getEntitiesOfRecursive(entityType, outEntities);
	}

	return int(outEntities.size());
}

Entity* GroupEntity::getFirstEntityOfRecursive(const Type& entityType) const
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

GroupEntity* GroupEntity::findChildGroup(const std::wstring& name) const
{
	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		GroupEntity* childGroup = dynamic_type_cast< GroupEntity* >(*i);
		if (!childGroup)
			continue;

		if (childGroup->m_name == name)
			return childGroup;
	}
	return 0;
}

GroupEntity* GroupEntity::findChildGroupRecursive(const std::wstring& name) const
{
	for (RefArray< Entity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		GroupEntity* childGroup = dynamic_type_cast< GroupEntity* >(*i);
		if (!childGroup)
			continue;

		if (childGroup->m_name == name)
			return childGroup;

		childGroup = childGroup->findChildGroupRecursive(name);
		if (childGroup)
			return childGroup;
	}
	return 0;
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

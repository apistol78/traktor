#include <algorithm>
#include "World/Entity/SpatialGroupEntity.h"
#include "Core/Misc/Save.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SpatialGroupEntity", SpatialGroupEntity, SpatialEntity)

SpatialGroupEntity::SpatialGroupEntity(const Transform& transform)
:	m_transform(transform)
,	m_update(false)
{
}

SpatialGroupEntity::~SpatialGroupEntity()
{
	destroy();
}

void SpatialGroupEntity::destroy()
{
	T_ASSERT (m_remove.empty());
	for (RefArray< SpatialEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_entities.resize(0);
}

void SpatialGroupEntity::addEntity(SpatialEntity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	T_ASSERT_M (!m_update, L"Not allowed to add entity during update");
	m_entities.push_back(entity);
}

void SpatialGroupEntity::removeEntity(SpatialEntity* entity)
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
		RefArray< SpatialEntity >::iterator i = std::find(m_entities.begin(), m_entities.end(), entity);
		m_entities.erase(i);
	}
}

void SpatialGroupEntity::removeAllEntities()
{
	T_ASSERT_M (!m_update, L"Cannot remove all entities while in update; not implemented");
	m_entities.resize(0);
}
	
const RefArray< SpatialEntity >& SpatialGroupEntity::getEntities() const
{
	return m_entities;
}
	
int SpatialGroupEntity::getEntitiesOf(const TypeInfo& entityType, RefArray< SpatialEntity >& entities) const
{
	for (RefArray< SpatialEntity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (is_type_of(entityType, type_of(*i)))
			entities.push_back(*i);
	}
	return int(entities.size());
}

Ref< SpatialEntity > SpatialGroupEntity::getFirstEntityOf(const TypeInfo& entityType) const
{
	for (RefArray< SpatialEntity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (is_type_of(entityType, type_of(*i)))
			return *i;
	}
	return 0;
}
	
void SpatialGroupEntity::update(const EntityUpdate* update)
{
	// Update child entities; set flag to indicate we're
	// updating 'em.
	{
		Save< bool > scope(m_update, true);
		for (RefArray< SpatialEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
			(*i)->update(update);
	}
	// Remove deferred removed entities.
	if (!m_remove.empty())
	{
		for (RefArray< SpatialEntity >::iterator i = m_remove.begin(); i != m_remove.end(); ++i)
			removeEntity(*i);
		m_remove.resize(0);
	}
}

void SpatialGroupEntity::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.inverse();
	for (RefArray< SpatialEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Transform currentTransform;
		if ((*i)->getTransform(currentTransform))
		{
			Transform Tlocal = invTransform * currentTransform;
			Transform Tworld = transform * Tlocal;
			(*i)->setTransform(Tworld);
		}
	}
	m_transform = transform;
}

bool SpatialGroupEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 SpatialGroupEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (RefArray< SpatialEntity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Aabb3 childBoundingBox = (*i)->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform;
			(*i)->getTransform(childTransform);

			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

	}
}

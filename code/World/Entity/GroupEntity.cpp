#include <algorithm>
#include "Core/Misc/Save.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntity", GroupEntity, Entity)

GroupEntity::GroupEntity()
:	m_transform(Transform::identity())
,	m_mask(EmAll)
,	m_update(false)
{
}

GroupEntity::GroupEntity(const Transform& transform, uint32_t mask)
:	m_transform(transform)
,	m_mask(mask)
,	m_update(false)
{
}

GroupEntity::~GroupEntity()
{
}

void GroupEntity::destroy()
{
	T_ASSERT(m_deferred[0].empty());
	T_ASSERT(m_deferred[1].empty());
	for (auto entity : m_entities)
	{
		if (entity)
			entity->destroy();
	}
	m_entities.resize(0);
}

void GroupEntity::setMask(uint32_t mask)
{
	m_mask = mask;
}

void GroupEntity::addEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	if (m_update)
	{
		// Add as deferred add; cannot add while in update loop.
		m_deferred[0].push_back(entity);
	}
	else
		m_entities.push_back(entity);
}

void GroupEntity::removeEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	if (m_update)
	{
		// Add as deferred remove; we cannot remove while update
		// is iterating entity array.
		m_deferred[1].push_back(entity);
	}
	else
	{
		auto i = std::find(m_entities.begin(), m_entities.end(), entity);
		if (i != m_entities.end())
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

int GroupEntity::getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& entities) const
{
	for (auto entity : m_entities)
	{
		if (is_type_of(entityType, type_of(entity)))
			entities.push_back(entity);
	}
	return int(entities.size());
}

Entity* GroupEntity::getFirstEntityOf(const TypeInfo& entityType) const
{
	for (auto entity : m_entities)
	{
		if (is_type_of(entityType, type_of(entity)))
			return entity;
	}
	return nullptr;
}

void GroupEntity::update(const UpdateParams& update)
{
	// Update child entities; set flag to indicate we're
	// updating 'em.
	{
		T_ANONYMOUS_VAR(Save< bool >)(m_update, true);
		for (auto entity : m_entities)
			entity->update(update);
	}

	// Add deferred entities.
	if (!m_deferred[1].empty())
	{
		m_entities.insert(m_entities.end(), m_deferred[0].begin(), m_deferred[0].end());
		m_deferred[0].resize(0);
	}

	// Remove deferred entities.
	if (!m_deferred[1].empty())
	{
		for (auto entity : m_deferred[1])
			removeEntity(entity);
		m_deferred[1].resize(0);
	}
}

void GroupEntity::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.inverse();
	for (auto entity : m_entities)
	{
		Transform currentTransform;
		if (entity->getTransform(currentTransform))
		{
			Transform Tlocal = invTransform * currentTransform;
			Transform Tworld = transform * Tlocal;
			entity->setTransform(Tworld);
		}
	}
	m_transform = transform;
}

bool GroupEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 GroupEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (auto entity : m_entities)
	{
		Aabb3 childBoundingBox = entity->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform;
			entity->getTransform(childTransform);

			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

	}
}

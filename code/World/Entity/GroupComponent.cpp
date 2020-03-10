#include "Core/Misc/Save.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupComponent", GroupComponent, IEntityComponent)

GroupComponent::GroupComponent()
:	m_owner(nullptr)
,	m_update(false)
{
}

void GroupComponent::destroy()
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

void GroupComponent::setOwner(ComponentEntity* owner)
{
	T_ASSERT(m_owner == nullptr);
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void GroupComponent::update(const UpdateParams& update)
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

void GroupComponent::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.inverse();
	for (auto entity : m_entities)
	{
		Transform currentTransform = entity->getTransform();
		Transform Tlocal = invTransform * currentTransform;
		Transform Tworld = transform * Tlocal;
		entity->setTransform(Tworld);
	}
	m_transform = transform;
}

Aabb3 GroupComponent::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (auto entity : m_entities)
	{
		Aabb3 childBoundingBox = entity->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform = entity->getTransform();
			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

void GroupComponent::addEntity(Entity* entity)
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

void GroupComponent::removeEntity(Entity* entity)
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

void GroupComponent::removeAllEntities()
{
	T_ASSERT_M (!m_update, L"Cannot remove all entities while in update; not implemented.");
	m_entities.resize(0);
}

const RefArray< Entity >& GroupComponent::getEntities() const
{
	return m_entities;
}

int32_t GroupComponent::getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& entities) const
{
	for (auto entity : m_entities)
	{
		if (is_type_of(entityType, type_of(entity)))
			entities.push_back(entity);
	}
	return (int32_t)entities.size();
}

Entity* GroupComponent::getFirstEntityOf(const TypeInfo& entityType) const
{
	for (auto entity : m_entities)
	{
		if (is_type_of(entityType, type_of(entity)))
			return entity;
	}
	return nullptr;
}

	}
}

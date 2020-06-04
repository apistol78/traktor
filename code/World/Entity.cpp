#include "World/Entity.h"
#include "World/IEntityComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.Entity", Entity, Object)

Entity::Entity()
:	m_transform(Transform::identity())
{
}

Entity::Entity(const Transform& transform)
:	m_transform(transform)
{
}

Entity::Entity(const Transform& transform, const RefArray< IEntityComponent >& components)
:	m_transform(transform)
,	m_components(components)
{
	for (auto component : m_components)
	{
		component->setOwner(this);
		component->setTransform(m_transform);
	}
}

Entity::~Entity()
{
	destroy();
}

void Entity::destroy()
{
	for (auto component : m_components)
		component->destroy();
	m_components.clear();
}

void Entity::setTransform(const Transform& transform)
{
	if (transform != m_transform)
	{
		m_transform = transform;
		for (auto component : m_components)
			component->setTransform(transform);
	}
}

Transform Entity::getTransform() const
{
	return m_transform;
}

Aabb3 Entity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (auto component : m_components)
		boundingBox.contain(component->getBoundingBox());
	return boundingBox;
}

void Entity::update(const UpdateParams& update)
{
	for (auto component : m_components)
		component->update(update);
}

void Entity::setComponent(IEntityComponent* component)
{
	T_FATAL_ASSERT (component);

	component->setOwner(this);
	component->setTransform(m_transform);

	// Replace existing component of same type.
	for (auto comp : m_components)
	{
		if (is_type_of(type_of(comp), type_of(component)))
		{
			comp = component;
			return;
		}
	}

	// No such component, add last.
	m_components.push_back(component);
}

IEntityComponent* Entity::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
	{
		if (is_type_of(componentType, type_of(component)))
			return component;
	}
	return nullptr;
}

	}
}

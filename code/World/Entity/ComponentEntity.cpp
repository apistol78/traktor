#include "World/IEntityComponent.h"
#include "World/Entity/ComponentEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntity", ComponentEntity, Entity)

ComponentEntity::ComponentEntity()
:	m_transform(Transform::identity())
{
}

ComponentEntity::ComponentEntity(const Transform& transform)
:	m_transform(transform)
{
}

ComponentEntity::~ComponentEntity()
{
	destroy();
}

void ComponentEntity::destroy()
{
	for (auto component : m_components)
		component->destroy();
	m_components.clear();
}

void ComponentEntity::setTransform(const Transform& transform)
{
	if (transform != m_transform)
	{
		m_transform = transform;
		for (auto component : m_components)
			component->setTransform(transform);
	}
}

bool ComponentEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 ComponentEntity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (auto component : m_components)
		boundingBox.contain(component->getBoundingBox());
	return boundingBox;
}

void ComponentEntity::update(const UpdateParams& update)
{
	for (auto component : m_components)
		component->update(update);
}

void ComponentEntity::setComponent(IEntityComponent* component)
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

IEntityComponent* ComponentEntity::getComponent(const TypeInfo& componentType) const
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

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/IEntityComponent.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
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

void ComponentEntity::destroy()
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->destroy();
	m_components.clear();
}

void ComponentEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->setTransform(transform);
}

bool ComponentEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 ComponentEntity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		boundingBox.contain((*i)->getBoundingBox());
	return boundingBox;
}

void ComponentEntity::update(const UpdateParams& update)
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->update(update);
}

void ComponentEntity::setComponent(IEntityComponent* component)
{
	T_FATAL_ASSERT (component);

	component->setOwner(this);
	component->setTransform(m_transform);

	// Replace existing component of same type.
	for (RefArray< IEntityComponent >::iterator i = m_components.begin(); i != m_components.end(); ++i)
	{
		if (is_type_of(type_of(*i), type_of(component)))
		{
			*i = component;
			return;
		}
	}

	// No such component, add last.
	m_components.push_back(component);
}

IEntityComponent* ComponentEntity::getComponent(const TypeInfo& componentType) const
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
	{
		if (is_type_of(componentType, type_of(*i)))
			return *i;
	}
	return 0;
}

void ComponentEntity::render(WorldContext& worldContext, WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass)
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		worldContext.build(worldRenderView, worldRenderPass, *i);
}

	}
}

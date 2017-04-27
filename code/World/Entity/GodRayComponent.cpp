/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/Entity.h"
#include "World/Entity/GodRayComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GodRayComponent", GodRayComponent, IEntityComponent)

GodRayComponent::GodRayComponent()
:	m_owner(0)
{
}

void GodRayComponent::destroy()
{
}

void GodRayComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void GodRayComponent::update(const UpdateParams& update)
{
}

void GodRayComponent::setTransform(const Transform& transform)
{
}

Aabb3 GodRayComponent::getBoundingBox() const
{
	return Aabb3();
}

Transform GodRayComponent::getTransform() const
{
	Transform transform;
	m_owner->getTransform(transform);
	return transform;
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/Entity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.Entity", Entity, Object)

Entity::~Entity()
{
}

void Entity::destroy()
{
	// Nothing to destroy.
}

void Entity::setTransform(const Transform& transform)
{
	// Don't do anything.
}

bool Entity::getTransform(Transform& outTransform) const
{
	// Doesn't have a transform.
	return false;
}

Aabb3 Entity::getWorldBoundingBox() const
{
	Aabb3 boundingBox = getBoundingBox();
	if (boundingBox.empty())
		return Aabb3();

	Transform transform;
	return getTransform(transform) ? boundingBox.transform(transform) : boundingBox;
}

	}
}

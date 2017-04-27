/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Joint.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntity", ArticulatedEntity, world::Entity)

ArticulatedEntity::ArticulatedEntity(
	const Transform& transform,
	const RefArray< RigidEntity >& entities,
	const RefArray< Joint >& joints
)
:	m_transform(transform)
,	m_entities(entities)
,	m_joints(joints)
{
}

void ArticulatedEntity::destroy()
{
	for (RefArray< Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_joints.resize(0);

	for (RefArray< RigidEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_entities.resize(0);

	world::Entity::destroy();
}

void ArticulatedEntity::update(const world::UpdateParams& update)
{
	for (RefArray< RigidEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
		(*i)->update(update);
}

void ArticulatedEntity::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.inverse();
	for (RefArray< RigidEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
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

bool ArticulatedEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 ArticulatedEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (RefArray< RigidEntity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Aabb3 childBoundingBox = (*i)->getWorldBoundingBox();
		if (!childBoundingBox.empty())
			boundingBox.contain(childBoundingBox.transform(invTransform));
	}

	return boundingBox;
}

	}
}

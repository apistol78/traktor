#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/Joint.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntity", ArticulatedEntity, world::SpatialEntity)

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

	world::SpatialEntity::destroy();
}

void ArticulatedEntity::update(const world::EntityUpdate* update)
{
	for (RefArray< RigidEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
		(*i)->update(update);
}

void ArticulatedEntity::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * m_transform.inverse();
	for (RefArray< RigidEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Transform currentTransform;
		if ((*i)->getTransform(currentTransform))
			(*i)->setTransform(deltaTransform * currentTransform);
	}
	m_transform = transform;
}

bool ArticulatedEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb ArticulatedEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb boundingBox;
	for (RefArray< RigidEntity >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Aabb childBoundingBox = (*i)->getWorldBoundingBox();
		if (!childBoundingBox.empty())
			boundingBox.contain(childBoundingBox.transform(invTransform));
	}

	return boundingBox;
}

	}
}

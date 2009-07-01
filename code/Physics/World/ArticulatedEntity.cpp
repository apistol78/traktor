#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/Joint.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntity", ArticulatedEntity, world::SpatialEntity)

ArticulatedEntity::ArticulatedEntity(
	const Matrix44& transform,
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

void ArticulatedEntity::setTransform(const Matrix44& transform)
{
	Matrix44 deltaTransform = m_transform.inverseOrtho() * transform;
	for (RefArray< RigidEntity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Matrix44 currentTransform;
		if ((*i)->getTransform(currentTransform))
			(*i)->setTransform(currentTransform * deltaTransform);
	}
	m_transform = transform;
}

bool ArticulatedEntity::getTransform(Matrix44& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb ArticulatedEntity::getBoundingBox() const
{
	Matrix44 invTransform = m_transform.inverseOrtho();

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

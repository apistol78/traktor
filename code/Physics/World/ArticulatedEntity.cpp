#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/Joint.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntity", ArticulatedEntity, world::SpatialGroupEntity)

ArticulatedEntity::ArticulatedEntity(
	const Matrix44& transform,
	const RefArray< RigidEntity >& entities,
	const RefArray< Joint >& joints
)
:	world::SpatialGroupEntity(transform)
,	m_joints(joints)
{
	for (RefArray< RigidEntity >::const_iterator i = entities.begin(); i != entities.end(); ++i)
		addEntity(*i);
}

void ArticulatedEntity::destroy()
{
	for (RefArray< Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_joints.resize(0);

	world::SpatialGroupEntity::destroy();
}

	}
}

#ifndef traktor_physics_ConeTwistJointPhysX_H
#define traktor_physics_ConeTwistJointPhysX_H

#include "Physics/ConeTwistJoint.h"
#include "Physics/PhysX/JointPhysX.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS ConeTwistJointPhysX : public JointPhysX< ConeTwistJoint >
{
	T_RTTI_CLASS;

public:
	ConeTwistJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2);
};

	}
}

#endif	// traktor_physics_ConeTwistJointPhysX_H

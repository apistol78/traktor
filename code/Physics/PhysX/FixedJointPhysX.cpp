#include "Physics/PhysX/FixedJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.FixedJointPhysX", FixedJointPhysX, FixedJoint)

FixedJointPhysX::FixedJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< FixedJoint >(callback, joint, body1, body2)
{
}

	}
}

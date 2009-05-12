#include "Physics/PhysX/ConeTwistJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ConeTwistJointPhysX", ConeTwistJointPhysX, ConeTwistJoint)

ConeTwistJointPhysX::ConeTwistJointPhysX(DestroyCallbackPhysX* callback, NxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< ConeTwistJoint >(callback, joint, body1, body2)
{
}

	}
}

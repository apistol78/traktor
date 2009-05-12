#include "Physics/PhysX/BallJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointPhysX", BallJointPhysX, BallJoint)

BallJointPhysX::BallJointPhysX(DestroyCallbackPhysX* callback, NxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< BallJoint >(callback, joint, body1, body2)
{
}

Vector4 BallJointPhysX::getAnchor() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

	}
}

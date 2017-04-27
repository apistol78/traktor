/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/PhysX/ConeTwistJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ConeTwistJointPhysX", ConeTwistJointPhysX, ConeTwistJoint)

ConeTwistJointPhysX::ConeTwistJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< ConeTwistJoint >(callback, joint, body1, body2)
{
}

	}
}

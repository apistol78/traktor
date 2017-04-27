/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Havok/ConeTwistJointHavok.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ConeTwistJointHavok", ConeTwistJointHavok, ConeTwistJoint)

ConeTwistJointHavok::ConeTwistJointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2)
:	JointHavok< ConeTwistJoint >(callback, constraint, body1, body2)
{
}

	}
}

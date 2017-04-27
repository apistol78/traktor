/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Havok/BallJointHavok.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointHavok", BallJointHavok, BallJoint)

BallJointHavok::BallJointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2)
:	JointHavok< BallJoint >(callback, constraint, body1, body2)
{
}

void BallJointHavok::setAnchor(const Vector4& anchor)
{
}

Vector4 BallJointHavok::getAnchor() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Havok/HingeJointHavok.h"
#include "Physics/Havok/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointHavok", HingeJointHavok, HingeJoint)

HingeJointHavok::HingeJointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2)
:	JointHavok< HingeJoint >(callback, constraint, body1, body2)
{
}

Vector4 HingeJointHavok::getAnchor() const
{
	return Vector4::origo();
}

Vector4 HingeJointHavok::getAxis() const
{
	return Vector4::zero();
}

float HingeJointHavok::getAngle() const
{
	return 0.0f;
}

float HingeJointHavok::getAngleVelocity() const
{
	return 0.0f;
}

void HingeJointHavok::setMotor(float targetVelocity, float maxImpulse)
{
}

	}
}

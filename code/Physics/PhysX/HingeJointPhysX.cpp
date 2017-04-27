/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <extensions\PxRevoluteJoint.h>
#include "Core/Math/Const.h"
#include "Physics/PhysX/HingeJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointPhysX", HingeJointPhysX, HingeJoint)

HingeJointPhysX::HingeJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< HingeJoint >(callback, joint, body1, body2)
{
}

Vector4 HingeJointPhysX::getAnchor() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

Vector4 HingeJointPhysX::getAxis() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

float HingeJointPhysX::getAngle() const
{
	physx::PxRevoluteJoint* revoluteJoint = static_cast< physx::PxRevoluteJoint* >(m_joint);
	return revoluteJoint->getAngle();
}

float HingeJointPhysX::getAngleVelocity() const
{
	physx::PxRevoluteJoint* revoluteJoint = static_cast< physx::PxRevoluteJoint* >(m_joint);
	return revoluteJoint->getVelocity();
}

void HingeJointPhysX::setMotor(float targetVelocity, float maxImpulse)
{
	physx::PxRevoluteJoint* revoluteJoint = static_cast< physx::PxRevoluteJoint* >(m_joint);
	if (maxImpulse > FUZZY_EPSILON)
	{
		revoluteJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		revoluteJoint->setConstraintFlag(physx::PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, false);
		revoluteJoint->setDriveVelocity(-targetVelocity);
		revoluteJoint->setDriveForceLimit(maxImpulse);
	}
	else
		revoluteJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, false);
}

	}
}

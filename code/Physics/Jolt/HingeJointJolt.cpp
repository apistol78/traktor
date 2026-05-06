/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Physics/Jolt/Conversion.h"
#include "Physics/Jolt/HingeJointJolt.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointJolt", HingeJointJolt, HingeJoint)

HingeJointJolt::HingeJointJolt(IWorldCallback* callback, JPH::HingeConstraint* constraint, BodyJolt* body1, BodyJolt* body2)
:	JointJolt< HingeJoint, JPH::HingeConstraint >(callback, constraint, body1, body2)
{
}

Vector4 HingeJointJolt::getAnchor() const
{
	if (!m_constraint || !m_body1)
		return Vector4::zero();
	const JPH::RMat44 com = m_body1->getJBody()->GetCenterOfMassTransform();
	return convertFromJolt(com * m_constraint->GetLocalSpacePoint1(), 1.0f);
}

Vector4 HingeJointJolt::getAxis() const
{
	if (!m_constraint || !m_body1)
		return Vector4::zero();
	const JPH::Vec3 axisLocal = m_constraint->GetLocalSpaceHingeAxis1();
	const JPH::RMat44 com = m_body1->getJBody()->GetCenterOfMassTransform();
	return convertFromJolt(com.Multiply3x3(axisLocal), 0.0f);
}

float HingeJointJolt::getAngle() const
{
	return m_constraint ? m_constraint->GetCurrentAngle() : 0.0f;
}

float HingeJointJolt::getAngleVelocity() const
{
	return 0.0f;
}

void HingeJointJolt::setMotor(float targetVelocity, float maxImpulse)
{
	if (!m_constraint)
		return;
	if (maxImpulse > FUZZY_EPSILON)
	{
		JPH::MotorSettings& ms = m_constraint->GetMotorSettings();
		ms.SetTorqueLimit(maxImpulse);
		m_constraint->SetTargetAngularVelocity(targetVelocity);
		m_constraint->SetMotorState(JPH::EMotorState::Velocity);
	}
	else
	{
		m_constraint->SetMotorState(JPH::EMotorState::Off);
	}
}

}

/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/Conversion.h"
#include "Physics/Jolt/Hinge2JointJolt.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Hinge2JointJolt", Hinge2JointJolt, Hinge2Joint)

Hinge2JointJolt::Hinge2JointJolt(IWorldCallback* callback, JPH::SixDOFConstraint* constraint, BodyJolt* body1, BodyJolt* body2)
:	JointJolt< Hinge2Joint, JPH::SixDOFConstraint >(callback, constraint, body1, body2)
{
}

void Hinge2JointJolt::addTorques(float torqueAxis1, float torqueAxis2)
{
}

float Hinge2JointJolt::getAngleAxis1() const
{
	return 0.0f;
}

void Hinge2JointJolt::setVelocityAxis1(float velocityAxis1)
{
	if (!m_constraint)
		return;
	JPH::Vec3 av = m_constraint->GetTargetAngularVelocityCS();
	av.SetX(velocityAxis1);
	m_constraint->SetTargetAngularVelocityCS(av);
	m_constraint->SetMotorState(JPH::SixDOFConstraint::EAxis::RotationX, JPH::EMotorState::Velocity);
}

void Hinge2JointJolt::setVelocityAxis2(float velocityAxis2)
{
	if (!m_constraint)
		return;
	JPH::Vec3 av = m_constraint->GetTargetAngularVelocityCS();
	av.SetY(velocityAxis2);
	m_constraint->SetTargetAngularVelocityCS(av);
	m_constraint->SetMotorState(JPH::SixDOFConstraint::EAxis::RotationY, JPH::EMotorState::Velocity);
}

void Hinge2JointJolt::getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const
{
	outAnchor1 = Vector4::zero();
	outAnchor2 = Vector4::zero();
}

}

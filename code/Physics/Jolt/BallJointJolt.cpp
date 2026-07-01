/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/BallJointJolt.h"
#include "Physics/Jolt/Conversion.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointJolt", BallJointJolt, BallJoint)

BallJointJolt::BallJointJolt(IWorldCallback* callback, JPH::PointConstraint* constraint, BodyJolt* body1, BodyJolt* body2)
:	JointJolt< BallJoint, JPH::PointConstraint >(callback, constraint, body1, body2)
{
}

void BallJointJolt::setAnchor(const Vector4& anchor)
{
	if (!m_constraint)
		return;
	m_constraint->SetPoint1(JPH::EConstraintSpace::WorldSpace, convertToJolt(anchor));
	m_constraint->SetPoint2(JPH::EConstraintSpace::WorldSpace, convertToJolt(anchor));
}

Vector4 BallJointJolt::getAnchor() const
{
	if (!m_constraint || !m_body1)
		return Vector4::zero();
	const JPH::RMat44 com = m_body1->getJBody()->GetCenterOfMassTransform();
	return convertFromJolt(com * m_constraint->GetLocalSpacePoint1(), 1.0f);
}

}

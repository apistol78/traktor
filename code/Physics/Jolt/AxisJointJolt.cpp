/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/AxisJointJolt.h"
#include "Physics/Jolt/Conversion.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.AxisJointJolt", AxisJointJolt, AxisJoint)

AxisJointJolt::AxisJointJolt(IWorldCallback* callback, JPH::HingeConstraint* constraint, BodyJolt* body1, BodyJolt* body2)
:	JointJolt< AxisJoint, JPH::HingeConstraint >(callback, constraint, body1, body2)
{
}

Vector4 AxisJointJolt::getAxis() const
{
	if (!m_constraint || !m_body1)
		return Vector4::zero();
	const JPH::Vec3 axisLocal = m_constraint->GetLocalSpaceHingeAxis1();
	const JPH::RMat44 com = m_body1->getJBody()->GetCenterOfMassTransform();
	return convertFromJolt(com.Multiply3x3(axisLocal), 0.0f);
}

}

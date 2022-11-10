/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Bullet/HingeJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointBullet", HingeJointBullet, HingeJoint)

HingeJointBullet::HingeJointBullet(IWorldCallback* callback, btHingeConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< HingeJoint, btHingeConstraint >(callback, constraint, body1, body2)
{
}

Vector4 HingeJointBullet::getAnchor() const
{
	return fromBtVector3(m_constraint->getAFrame().getOrigin(), 1.0f);
}

Vector4 HingeJointBullet::getAxis() const
{
	return fromBtVector3(m_constraint->getAFrame().getBasis().getColumn(2), 0.0f);
}

float HingeJointBullet::getAngle() const
{
	return m_constraint->getHingeAngle();
}

float HingeJointBullet::getAngleVelocity() const
{
	return 0.0f;
}

void HingeJointBullet::setMotor(float targetVelocity, float maxImpulse)
{
	if (maxImpulse > FUZZY_EPSILON)
		m_constraint->enableAngularMotor(true, targetVelocity, maxImpulse);
	else
		m_constraint->enableMotor(false);
}

	}
}

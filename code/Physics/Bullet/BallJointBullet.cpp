/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Bullet/BallJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointBullet", BallJointBullet, BallJoint)

BallJointBullet::BallJointBullet(IWorldCallback* callback, btPoint2PointConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< BallJoint, btPoint2PointConstraint >(callback, constraint, body1, body2)
{
}

void BallJointBullet::setAnchor(const Vector4& anchor)
{
	const btVector3 pivotInWorld = toBtVector3(anchor);
	const btVector3 pivotInA = m_constraint->getRigidBodyA().getWorldTransform().inverse() * pivotInWorld;
	const btVector3 pivotInB = m_constraint->getRigidBodyB().getWorldTransform().inverse() * pivotInWorld;
	m_constraint->setPivotA(pivotInA);
	m_constraint->setPivotB(pivotInB);
}

Vector4 BallJointBullet::getAnchor() const
{
	const btVector3& pivotInA = m_constraint->getPivotInA();
	const btVector3 pivotInWorld = m_constraint->getRigidBodyA().getWorldTransform() * pivotInA;
	return fromBtVector3(pivotInWorld, 1.0f);
}

}

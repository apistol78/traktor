/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Bullet/BallJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointBullet", BallJointBullet, BallJoint)

BallJointBullet::BallJointBullet(IWorldCallback* callback, btPoint2PointConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< BallJoint, btPoint2PointConstraint >(callback, constraint, body1, body2)
{
}

void BallJointBullet::setAnchor(const Vector4& anchor)
{
	btVector3 pivotInWorld = toBtVector3(anchor);
	btVector3 pivotInA = m_constraint->getRigidBodyA().getWorldTransform().inverse() * pivotInWorld;
	m_constraint->setPivotA(pivotInA);
	btVector3 pivotInB = m_constraint->getRigidBodyB().getWorldTransform().inverse() * pivotInWorld;
	m_constraint->setPivotB(pivotInB);
}

Vector4 BallJointBullet::getAnchor() const
{
	const btVector3& pivotInA = m_constraint->getPivotInA();
	btVector3 pivotInWorld = m_constraint->getRigidBodyA().getWorldTransform() * pivotInA;
	return fromBtVector3(pivotInWorld, 1.0f);
}

	}
}

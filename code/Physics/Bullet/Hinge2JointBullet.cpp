/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/Hinge2JointBullet.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Hinge2JointBullet", Hinge2JointBullet, Hinge2Joint)

Hinge2JointBullet::Hinge2JointBullet(IWorldCallback* callback, btHinge2Constraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< Hinge2Joint, btHinge2Constraint >(callback, constraint, body1, body2)
{
}

void Hinge2JointBullet::addTorques(float torqueAxis1, float torqueAxis2)
{
}

float Hinge2JointBullet::getAngleAxis1() const
{
	return 0.0f;
}

void Hinge2JointBullet::setVelocityAxis1(float velocityAxis1)
{
}

void Hinge2JointBullet::setVelocityAxis2(float velocityAxis2)
{
}

void Hinge2JointBullet::getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const
{
}

	}
}

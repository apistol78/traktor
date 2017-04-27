/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Bullet/FixedJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.FixedJointBullet", FixedJointBullet, FixedJoint)

FixedJointBullet::FixedJointBullet(IWorldCallback* callback, btGeneric6DofConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< FixedJoint, btGeneric6DofConstraint >(callback, constraint, body1, body2)
{
}

	}
}

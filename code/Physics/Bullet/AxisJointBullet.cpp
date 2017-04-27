/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Bullet/AxisJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.AxisJointBullet", AxisJointBullet, AxisJoint)

AxisJointBullet::AxisJointBullet(IWorldCallback* callback, btHingeConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< AxisJoint, btHingeConstraint >(callback, constraint, body1, body2)
{
}

Vector4 AxisJointBullet::getAxis() const
{
	return fromBtVector3(m_constraint->getAFrame().getBasis().getColumn(2), 0.0f);
}

	}
}

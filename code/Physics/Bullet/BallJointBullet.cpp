#include "Physics/Bullet/BallJointBullet.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointBullet", BallJointBullet, BallJoint)

BallJointBullet::BallJointBullet(DestroyCallback* callback, btPoint2PointConstraint* constraint, Body* body1, Body* body2)
:	JointBullet< BallJoint, btPoint2PointConstraint >(callback, constraint, body1, body2)
{
}

Vector4 BallJointBullet::getAnchor() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

	}
}

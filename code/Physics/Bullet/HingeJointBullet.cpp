#include "Physics/Bullet/HingeJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointBullet", HingeJointBullet, HingeJoint)

HingeJointBullet::HingeJointBullet(DestroyCallback* callback, btHingeConstraint* constraint, Body* body1, Body* body2)
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

	}
}

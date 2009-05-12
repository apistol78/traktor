#include "Physics/Ode/BallJointOde.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointOde", BallJointOde, BallJoint)

BallJointOde::BallJointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2)
:	JointOde< BallJoint >(callback, jointId, body1, body2)
{
}

Vector4 BallJointOde::getAnchor() const
{
	dVector3 anchor;
	dJointGetBallAnchor(m_jointId, anchor);
	return Vector4(anchor[0], anchor[1], anchor[2], 1.0f);
}

	}
}

#include "Physics/Ode/HingeJointOde.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointOde", HingeJointOde, HingeJoint)

HingeJointOde::HingeJointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2)
:	JointOde< HingeJoint >(callback, jointId, body1, body2)
{
}

Vector4 HingeJointOde::getAnchor() const
{
	dVector3 result;
	dJointGetHingeAnchor(m_jointId, result);
	return Vector4(result[0], result[1], result[2], 1.0f);
}

Vector4 HingeJointOde::getAxis() const
{
	dVector3 result;
	dJointGetHingeAxis(m_jointId, result);
	return Vector4(result[0], result[1], result[2], 0.0f);
}

float HingeJointOde::getAngle() const
{
	return dJointGetHingeAngle(m_jointId);
}

float HingeJointOde::getAngleVelocity() const
{
	return dJointGetHingeAngleRate(m_jointId);
}

	}
}

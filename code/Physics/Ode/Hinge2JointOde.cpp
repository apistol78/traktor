#include "Physics/Ode/Hinge2JointOde.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Hinge2JointOde", Hinge2JointOde, Hinge2Joint)

Hinge2JointOde::Hinge2JointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2)
:	JointOde< Hinge2Joint >(callback, jointId, body1, body2)
{
}

void Hinge2JointOde::addTorques(float torqueAxis1, float torqueAxis2)
{
	dJointAddHinge2Torques(m_jointId, torqueAxis1, torqueAxis2);
}

float Hinge2JointOde::getAngleAxis1() const
{
	return dJointGetHinge2Angle1(m_jointId);
}

void Hinge2JointOde::setVelocityAxis1(float velocityAxis1)
{
	dJointSetHinge2Param(m_jointId, dParamVel, velocityAxis1);
	dJointSetHinge2Param(m_jointId, dParamFMax, 10.0f);
	dJointSetHinge2Param(m_jointId, dParamFudgeFactor, 0.1f);
}

void Hinge2JointOde::setVelocityAxis2(float velocityAxis2)
{
	dJointSetHinge2Param(m_jointId, dParamVel2, velocityAxis2);
	dJointSetHinge2Param(m_jointId, dParamFMax2, 10.0f);
}

void Hinge2JointOde::getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const
{
	dVector3 anchor1, anchor2;
	dJointGetHinge2Anchor(m_jointId, anchor1);
	dJointGetHinge2Anchor2(m_jointId, anchor2);
	outAnchor1 = Vector4(anchor1[0], anchor1[1], anchor1[2], 1.0f);
	outAnchor2 = Vector4(anchor2[0], anchor2[1], anchor2[2], 1.0f);
}

	}
}

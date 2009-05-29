#include <limits>
#include "Physics/Ode/ConeTwistJointOde.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/DynamicBody.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const Vector4 c_axis[] =
{
	Vector4(1.0f, 0.0f, 0.0f, 0.0f),
	Vector4(0.0f, 1.0f, 0.0f, 0.0f),
	Vector4(0.0f, 0.0f, 1.0f, 0.0f)
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ConeTwistJointOde", ConeTwistJointOde, ConeTwistJoint)

ConeTwistJointOde::ConeTwistJointOde(DestroyCallback* callback, Body* body1, Body* body2, const ConeTwistJointDesc* desc)
:	JointOde< ConeTwistJoint >(callback, 0, body1, body2)
,	m_desc(desc)
{
	m_anchor1 = body1->getTransform().inverse() * m_desc->getAnchor();
	m_anchor2 = body2->getTransform().inverse() * m_desc->getAnchor();
	m_coneAxis1 = (body1->getTransform().inverse() * m_desc->getConeAxis()).normalized();
	m_coneAxis2 = (body2->getTransform().inverse() * m_desc->getConeAxis()).normalized();
	m_twistAxis1 = (body1->getTransform().inverse() * m_desc->getTwistAxis()).normalized();
	m_twistAxis2 = (body2->getTransform().inverse() * m_desc->getTwistAxis()).normalized();
}

Vector4 ConeTwistJointOde::getAnchor() const
{
	return m_body1->getTransform() * m_anchor1;
}

void ConeTwistJointOde::update(float deltaTime)
{
	const float c_tau = 0.3f;
	const float c_damping = 1.0f;

	Matrix44 tf1 = m_body1->getTransform();
	Matrix44 tf2 = m_body2->getTransform();
	Matrix44 tfInv1 = tf1.inverse();
	Matrix44 tfInv2 = tf2.inverse();

	Vector4 centerOfMass1 = tf1.translation();
	Vector4 centerOfMass2 = tf2.translation();

	Vector4 anchor1 = tf1 * m_anchor1;
	Vector4 anchor2 = tf2 * m_anchor2;
	Vector4 twistAxis1 = (tf1 * m_twistAxis1).normalized();
	Vector4 coneAxis1 = (tf1 * m_coneAxis1).normalized();
	Vector4 twistAxis2 = (tf2 * m_twistAxis2).normalized();
	Vector4 coneAxis2 = (tf2 * m_coneAxis2).normalized();

	float invMass1 = 0.0f;
	float invMass2 = 0.0f;
	Matrix33 inertiaInv1 = Matrix33::zero();
	Matrix33 inertiaInv2 = Matrix33::zero();
	Vector4 inertiaInvDiag1 = Vector4::zero();
	Vector4 inertiaInvDiag2 = Vector4::zero();
	Vector4 velocity1 = Vector4::zero();
	Vector4 velocity2 = Vector4::zero();
	Vector4 angularVelocity1 = Vector4::zero();
	Vector4 angularVelocity2 = Vector4::zero();

	if (is_a< DynamicBody >(m_body1))
	{
		invMass1 = static_cast< DynamicBody* >(m_body1.getPtr())->getInverseMass();
		inertiaInv1 = static_cast< DynamicBody* >(m_body1.getPtr())->getInertiaTensorInverseWorld();
		inertiaInvDiag1 = inertiaInv1.diagonal();
		velocity1 = static_cast< DynamicBody* >(m_body1.getPtr())->getVelocityAt(m_anchor1, true);
		angularVelocity1 = static_cast< DynamicBody* >(m_body1.getPtr())->getAngularVelocity();
	}
	if (is_a< DynamicBody >(m_body2))
	{
		invMass2 = static_cast< DynamicBody* >(m_body2.getPtr())->getInverseMass();
		inertiaInv2 = static_cast< DynamicBody* >(m_body2.getPtr())->getInertiaTensorInverseWorld();
		inertiaInvDiag2 = inertiaInv2.diagonal();
		velocity2 = static_cast< DynamicBody* >(m_body2.getPtr())->getVelocityAt(m_anchor2, true);
		angularVelocity2 = static_cast< DynamicBody* >(m_body2.getPtr())->getAngularVelocity();
	}

	// Solve point constraint.
	for (int i = 0; i < 3; ++i)
	{
		// Build Jacobian.
		Vector4 relPos1 = anchor1 - centerOfMass1;
		Vector4 relPos2 = anchor2 - centerOfMass2;
		if (
			relPos1.length2() <= FUZZY_EPSILON * FUZZY_EPSILON ||
			relPos2.length2() <= FUZZY_EPSILON * FUZZY_EPSILON
		)
			continue;

		Vector4 aJ = tfInv1 * cross(relPos1, c_axis[i]);
		Vector4 bJ = tfInv2 * cross(relPos2, -c_axis[i]);
		Vector4 MinvJt0 = inertiaInvDiag1 * aJ;
		Vector4 MinvJt1 = inertiaInvDiag2 * bJ;
		float jacDiagAB = invMass1 + dot3(MinvJt0, aJ) + invMass2 + dot3(MinvJt1, bJ);

		if (abs(jacDiagAB) < FUZZY_EPSILON)
			continue;

		// Solve constraint.
		float jacDiagABInv = 1.0f / jacDiagAB;

		float relativeVelocity = velocity1[i] - velocity2[i];	// Relative velocity on axis.
		float positionError = anchor2[i] - anchor1[i];			// Positional error on axis.

		float impulse = positionError * c_tau / deltaTime * jacDiagABInv - c_damping * relativeVelocity * jacDiagABInv;
		Vector4 impulseVector = c_axis[i] * Scalar(impulse);

		if (is_a< DynamicBody >(m_body1))
			static_cast< DynamicBody* >(m_body1.getPtr())->addImpulse(anchor1, impulseVector, false);
		if (is_a< DynamicBody >(m_body2))
			static_cast< DynamicBody* >(m_body2.getPtr())->addImpulse(anchor2, -impulseVector, false);
	}

	// Solve cone limit.
#if 1
	do
	{
		float coneAngleLimit1, coneAngleLimit2;
		m_desc->getConeAngles(coneAngleLimit1, coneAngleLimit2);

		Vector4 coneAxis2in1 = Quaternion(twistAxis2, twistAxis1) * coneAxis2;
		Vector4 coneAxis1ref = cross(coneAxis1, twistAxis1).normalized();
		float a = dot3(coneAxis1, coneAxis2in1);
		float b = dot3(coneAxis1ref, coneAxis2in1);
		float twistAngle = atan2f(a, b);

		float coneGradient = abs(sinf(twistAngle));
		float coneAngleLimit = coneAngleLimit1 * (1.0f - coneGradient) + coneAngleLimit2 * coneGradient;

		T_ASSERT (coneAngleLimit >= min(coneAngleLimit1, coneAngleLimit2) - FUZZY_EPSILON);
		T_ASSERT (coneAngleLimit <= max(coneAngleLimit1, coneAngleLimit2) + FUZZY_EPSILON);

		Vector4 v = (centerOfMass2 - anchor2).normalized();

		float coneAngle = dot3(v, twistAxis1);
		if (abs(coneAngle) > 1.0f - std::numeric_limits< float >::epsilon())
			continue;

		float coneAngleError = abs(acosf(coneAngle)) - coneAngleLimit / 2.0f;
		if (coneAngleError <= 0.0f)
			continue;

		Vector4 impulseAxis = cross(twistAxis1, v).normalized();

		// Impulse denominator.
		float angularImpulseDenom1 = dot3(inertiaInv1 * impulseAxis, impulseAxis);
		float angularImpulseDenom2 = dot3(inertiaInv2 * impulseAxis, impulseAxis);
		float kCone = 1.0f / (angularImpulseDenom1 + angularImpulseDenom2);

		// Compute impulse.
		Vector4 angularVelocity = angularVelocity2 - angularVelocity1;
		float amplitude = dot3(angularVelocity, impulseAxis) + (coneAngleError * c_tau) / deltaTime;
		float impulse = amplitude * kCone;

		// Apply impulses.
		if (is_a< DynamicBody >(m_body1))
			static_cast< DynamicBody* >(m_body1.getPtr())->addAngularImpulse(impulseAxis * Scalar(impulse), false);
		if (is_a< DynamicBody >(m_body2))
			static_cast< DynamicBody* >(m_body2.getPtr())->addAngularImpulse(impulseAxis * Scalar(-impulse), false);
	}
	while (false);
#endif

	// Solve twist limit.
#if 1
	do 
	{
		Vector4 coneAxis2in1 = Quaternion(twistAxis2, twistAxis1) * coneAxis2;
		Vector4 coneAxis1ref = cross(coneAxis1, twistAxis1).normalized();
		float a = dot3(coneAxis1, coneAxis2in1);
		float b = dot3(coneAxis1ref, coneAxis2in1);
		float twistAngle = atan2f(a, b) - PI / 2.0f;

		float twistAngleLimit = m_desc->getTwistAngle();
		float twistAngleError = abs(twistAngle) - twistAngleLimit / 2.0f;
		if (abs(twistAngleError) <= FUZZY_EPSILON)
			continue;

		if (twistAngle < 0.0f)
			twistAngleError = -twistAngleError;

		Vector4 impulseAxis = twistAxis2;

		// Impulse denominator.
		float angularImpulseDenom1 = dot3(inertiaInv1 * impulseAxis, impulseAxis);
		float angularImpulseDenom2 = dot3(inertiaInv2 * impulseAxis, impulseAxis);
		float kTwist = 1.0f / (angularImpulseDenom1 + angularImpulseDenom2);

		// Compute impulse.
		Vector4 angularVelocity = angularVelocity2 - angularVelocity1;
		float amplitude = dot3(angularVelocity, impulseAxis) * 1.0f + (twistAngleError * c_tau) / deltaTime;
		float impulse = amplitude * kTwist;

		// Apply impulses.
		if (is_a< DynamicBody >(m_body1))
			static_cast< DynamicBody* >(m_body1.getPtr())->addAngularImpulse(impulseAxis * Scalar(impulse), false);
		if (is_a< DynamicBody >(m_body2))
			static_cast< DynamicBody* >(m_body2.getPtr())->addAngularImpulse(impulseAxis * Scalar(-impulse), false);
	}
	while (false);
#endif
}

	}
}

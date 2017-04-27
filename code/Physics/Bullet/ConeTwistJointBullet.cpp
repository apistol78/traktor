/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Math/Quaternion.h"
#include "Core/Math/Const.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/Bullet/BodyBullet.h"
#include "Physics/Bullet/ConeTwistJointBullet.h"
#include "Physics/Bullet/Conversion.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ConeTwistJointBullet", ConeTwistJointBullet, ConeTwistJoint)

ConeTwistJointBullet::ConeTwistJointBullet(IWorldCallback* callback, JointConstraint* constraint, const Transform& transform, BodyBullet* body1, const ConeTwistJointDesc* desc)
:	JointBullet< ConeTwistJoint, JointConstraint >(callback, constraint, body1, 0)
,	m_desc(desc)
{
	Vector4 anchor = transform * m_desc->getAnchor().xyz1();
	Vector4 coneAxis = transform * m_desc->getConeAxis().xyz0();
	Vector4 twistAxis = transform * m_desc->getTwistAxis().xyz0();

	m_anchor1 = body1->getTransform().inverse() * anchor;
	m_coneAxis1 = (body1->getTransform().inverse() * coneAxis).normalized();
	m_twistAxis1 = (body1->getTransform().inverse() * twistAxis).normalized();

	m_anchor2 = anchor;
	m_coneAxis2 = coneAxis;
	m_twistAxis2 = twistAxis;

	m_dynamicBody1 = body1;
}

ConeTwistJointBullet::ConeTwistJointBullet(IWorldCallback* callback, JointConstraint* constraint, const Transform& transform, BodyBullet* body1, BodyBullet* body2, const ConeTwistJointDesc* desc)
:	JointBullet< ConeTwistJoint, JointConstraint >(callback, constraint, body1, body2)
,	m_desc(desc)
{
	Vector4 anchor = transform * m_desc->getAnchor().xyz1();
	Vector4 coneAxis = transform * m_desc->getConeAxis().xyz0();
	Vector4 twistAxis = transform * m_desc->getTwistAxis().xyz0();

	m_anchor1 = body1->getTransform().inverse() * anchor;
	m_coneAxis1 = (body1->getTransform().inverse() * coneAxis).normalized();
	m_twistAxis1 = (body1->getTransform().inverse() * twistAxis).normalized();

	m_anchor2 = body2->getTransform().inverse() * anchor;
	m_coneAxis2 = (body2->getTransform().inverse() * coneAxis).normalized();
	m_twistAxis2 = (body2->getTransform().inverse() * twistAxis).normalized();

	m_dynamicBody1 = body1;
	m_dynamicBody2 = body2;
}

void ConeTwistJointBullet::prepare()
{
	Transform tf1 = m_body1->getTransform();
	Transform tf2 = m_body2 ? m_body2->getTransform() : Transform::identity();
	Transform tfInv1 = tf1.inverse();
	Transform tfInv2 = tf2.inverse();

	Vector4 centerOfMass1 = tf1.translation().xyz1();
	Vector4 centerOfMass2 = tf2.translation().xyz1();

	Vector4 anchor1 = tf1 * m_anchor1;
	Vector4 anchor2 = tf2 * m_anchor2;
	Vector4 twistAxis1 = (tf1 * m_twistAxis1).normalized();
	Vector4 coneAxis1 = (tf1 * m_coneAxis1).normalized();
	Vector4 twistAxis2 = (tf2 * m_twistAxis2).normalized();
	Vector4 coneAxis2 = (tf2 * m_coneAxis2).normalized();

	Scalar invMass1(0.0f);
	Scalar invMass2(0.0f);
	Matrix33 inertiaInv1 = Matrix33::zero();
	Matrix33 inertiaInv2 = Matrix33::zero();
	Vector4 inertiaInvDiag1 = Vector4::zero();
	Vector4 inertiaInvDiag2 = Vector4::zero();

	if (m_dynamicBody1)
	{
		invMass1 = Scalar(m_dynamicBody1->getInverseMass());
		inertiaInv1 = m_dynamicBody1->getInertiaTensorInverseWorld();
		inertiaInvDiag1 = inertiaInv1.diagonal();
	}
	if (m_dynamicBody2)
	{
		invMass2 = Scalar(m_dynamicBody2->getInverseMass());
		inertiaInv2 = m_dynamicBody2->getInertiaTensorInverseWorld();
		inertiaInvDiag2 = inertiaInv2.diagonal();
	}

	// Point constraint.
	for (int i = 0; i < 3; ++i)
	{
		Vector4 relPos1 = anchor1 - centerOfMass1;
		Vector4 relPos2 = anchor2 - centerOfMass2;

		Vector4 aJ = tfInv1 * cross(relPos1, c_axis[i]);
		Vector4 bJ = tfInv2 * cross(relPos2, -c_axis[i]);
		Vector4 MinvJt0 = inertiaInvDiag1 * aJ;
		Vector4 MinvJt1 = inertiaInvDiag2 * bJ;
		Scalar diagAB = invMass1 + dot3(MinvJt0, aJ) + invMass2 + dot3(MinvJt1, bJ);

		m_jac[i].diagABInv = Scalar(1.0f) / diagAB;
	}

	// Cone constraint.
	float coneAngleLimit1, coneAngleLimit2;
	m_desc->getConeAngles(coneAngleLimit1, coneAngleLimit2);

	//float coneGradient = abs(sinf(twistAngle));
	m_coneAngleLimit = Scalar(coneAngleLimit1); //coneAngleLimit1 * (1.0f - coneGradient) + coneAngleLimit2 * coneGradient;
	//T_ASSERT (m_coneAngleLimit >= min(coneAngleLimit1, coneAngleLimit2) - FUZZY_EPSILON);
	//T_ASSERT (m_coneAngleLimit <= max(coneAngleLimit1, coneAngleLimit2) + FUZZY_EPSILON);

	m_coneImpulseAxis = cross(twistAxis1, twistAxis2);
	if (m_coneImpulseAxis.length() > FUZZY_EPSILON)
		m_coneImpulseAxis = m_coneImpulseAxis.normalized();
	else
		m_coneImpulseAxis = c_axis[0];

	Scalar angularImpulseDenom1 = dot3(inertiaInv1 * m_coneImpulseAxis, m_coneImpulseAxis);
	Scalar angularImpulseDenom2 = dot3(inertiaInv2 * m_coneImpulseAxis, m_coneImpulseAxis);
	m_kCone = Scalar(1.0f) / (angularImpulseDenom1 + angularImpulseDenom2);

	// Twist constraint.
	m_twistImpulseAxis = (twistAxis1 + twistAxis2) * Scalar(0.5f);

	angularImpulseDenom1 = dot3(inertiaInv1 * m_twistImpulseAxis, m_twistImpulseAxis);
	angularImpulseDenom2 = dot3(inertiaInv2 * m_twistImpulseAxis, m_twistImpulseAxis);
	m_kTwist = Scalar(1.0f) / (angularImpulseDenom1 + angularImpulseDenom2);
}

void ConeTwistJointBullet::update(float deltaTime)
{
	const Scalar c_tau(0.3f);
	const Scalar c_damping(1.0f);

	Transform tf1 = m_body1->getBodyTransform();
	Transform tf2 = m_body2 ? m_body2->getBodyTransform() : Transform::identity();

	Vector4 centerOfMass1 = tf1.translation().xyz1();
	Vector4 centerOfMass2 = tf2.translation().xyz1();

	Vector4 anchor1 = tf1 * m_anchor1;
	Vector4 anchor2 = tf2 * m_anchor2;
	Vector4 twistAxis1 = (tf1 * m_twistAxis1).normalized();
	Vector4 coneAxis1 = (tf1 * m_coneAxis1).normalized();
	Vector4 twistAxis2 = (tf2 * m_twistAxis2).normalized();
	Vector4 coneAxis2 = (tf2 * m_coneAxis2).normalized();

	Vector4 velocity1 = Vector4::zero();
	Vector4 velocity2 = Vector4::zero();
	Vector4 angularVelocity1 = Vector4::zero();
	Vector4 angularVelocity2 = Vector4::zero();

	if (m_dynamicBody1)
	{
		velocity1 = m_dynamicBody1->getVelocityAt(m_anchor1, true);
		angularVelocity1 = m_dynamicBody1->getAngularVelocity();
	}
	if (m_dynamicBody2)
	{
		velocity2 = m_dynamicBody2->getVelocityAt(m_anchor2, true);
		angularVelocity2 = m_dynamicBody2->getAngularVelocity();
	}

	Vector4 angularVelocity = angularVelocity2 - angularVelocity1;

	// Solve point constraint.
	Scalar tauDt = c_tau / Scalar(deltaTime);
	for (int i = 0; i < 3; ++i)
	{
		Scalar relativeVelocity = velocity2[i] - velocity1[i];	// Relative velocity on axis.
		Scalar positionError = anchor2[i] - anchor1[i];			// Positional error on axis.

		Scalar impulse = positionError * tauDt * m_jac[i].diagABInv + c_damping * relativeVelocity * m_jac[i].diagABInv;
		Vector4 impulseVector = c_axis[i] * impulse;

		if (m_dynamicBody1)
			m_dynamicBody1->addImpulse(anchor1, impulseVector, false);
		if (m_dynamicBody2)
			m_dynamicBody2->addImpulse(anchor2, -impulseVector, false);
	}

	// Solve cone constraint.
	do
	{
		Scalar phi = dot3(twistAxis1, twistAxis2);
		if (abs(phi) > 1.0f - std::numeric_limits< float >::epsilon())
			continue;

		Scalar coneAngle = Scalar(acosf(phi));
		Scalar coneAngleError = abs(coneAngle) - m_coneAngleLimit / Scalar(2.0f);
		if (coneAngleError <= 0.0f)
			continue;

		Scalar amplitude = dot3(angularVelocity, m_coneImpulseAxis) * c_damping + coneAngleError * tauDt;
		Scalar impulse = amplitude * m_kCone;

		if (m_dynamicBody1)
			m_dynamicBody1->addAngularImpulse(m_coneImpulseAxis * impulse, false);
		if (m_dynamicBody2)
			m_dynamicBody2->addAngularImpulse(m_coneImpulseAxis * -impulse, false);
	}
	while (false);

	// Solve twist constraint.
	Scalar twistAngleLimit = Scalar(m_desc->getTwistAngle());
	do 
	{
		Vector4 coneAxis2in1 = Quaternion(twistAxis2, twistAxis1) * coneAxis2;
		Vector4 coneAxis1ref = -cross(coneAxis1, twistAxis1).normalized();
		Scalar a = dot3(coneAxis1, coneAxis2in1);
		Scalar b = dot3(coneAxis1ref, coneAxis2in1);
		Scalar twistAngle = Scalar(atan2f(b, a));

		Scalar twistAngleError = abs(twistAngle) - twistAngleLimit / Scalar(2.0f);
		if (twistAngleError <= 0.0f)
			continue;

		if (twistAngle < 0.0f)
			twistAngleError = -twistAngleError;

		Scalar amplitude = dot3(angularVelocity, m_twistImpulseAxis) * c_damping + twistAngleError * tauDt;
		Scalar impulse = amplitude * m_kTwist;

		if (m_dynamicBody1)
			m_dynamicBody1->addAngularImpulse(m_twistImpulseAxis * impulse, false);
		if (m_dynamicBody2)
			m_dynamicBody2->addAngularImpulse(m_twistImpulseAxis * -impulse, false);
	}
	while (false);
}

	}
}

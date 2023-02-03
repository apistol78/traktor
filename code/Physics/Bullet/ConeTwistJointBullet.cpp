/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Quaternion.h"
#include "Core/Math/Const.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/Bullet/BodyBullet.h"
#include "Physics/Bullet/ConeTwistJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor::physics
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
	const Vector4 anchor = transform * m_desc->getAnchor().xyz1();
	const Vector4 coneAxis = transform * m_desc->getConeAxis().xyz0();
	const Vector4 twistAxis = transform * m_desc->getTwistAxis().xyz0();

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
	const Vector4 anchor = transform * m_desc->getAnchor().xyz1();
	const Vector4 coneAxis = transform * m_desc->getConeAxis().xyz0();
	const Vector4 twistAxis = transform * m_desc->getTwistAxis().xyz0();

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
	const Transform tf1 = m_body1->getTransform();
	const Transform tf2 = m_body2 ? m_body2->getTransform() : Transform::identity();
	const Transform tfInv1 = tf1.inverse();
	const Transform tfInv2 = tf2.inverse();

	const Vector4 centerOfMass1 = tf1.translation().xyz1();
	const Vector4 centerOfMass2 = tf2.translation().xyz1();

	const Vector4 anchor1 = tf1 * m_anchor1;
	const Vector4 anchor2 = tf2 * m_anchor2;
	const Vector4 twistAxis1 = (tf1 * m_twistAxis1).normalized();
	const Vector4 coneAxis1 = (tf1 * m_coneAxis1).normalized();
	const Vector4 twistAxis2 = (tf2 * m_twistAxis2).normalized();
	const Vector4 coneAxis2 = (tf2 * m_coneAxis2).normalized();

	Scalar invMass1 = 0.0_simd;
	Scalar invMass2 = 0.0_simd;
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
		const Vector4 relPos1 = anchor1 - centerOfMass1;
		const Vector4 relPos2 = anchor2 - centerOfMass2;

		const Vector4 aJ = tfInv1 * cross(relPos1, c_axis[i]);
		const Vector4 bJ = tfInv2 * cross(relPos2, -c_axis[i]);
		const Vector4 MinvJt0 = inertiaInvDiag1 * aJ;
		const Vector4 MinvJt1 = inertiaInvDiag2 * bJ;
		const Scalar diagAB = invMass1 + dot3(MinvJt0, aJ) + invMass2 + dot3(MinvJt1, bJ);

		m_jac[i].diagABInv = 1.0_simd / diagAB;
	}

	// Cone constraint.
	float coneAngleLimit1, coneAngleLimit2;
	m_desc->getConeAngles(coneAngleLimit1, coneAngleLimit2);

	//float coneGradient = abs(sinf(twistAngle));
	m_coneAngleLimit = Scalar(coneAngleLimit1); //coneAngleLimit1 * (1.0f - coneGradient) + coneAngleLimit2 * coneGradient;
	//T_ASSERT(m_coneAngleLimit >= min(coneAngleLimit1, coneAngleLimit2) - FUZZY_EPSILON);
	//T_ASSERT(m_coneAngleLimit <= max(coneAngleLimit1, coneAngleLimit2) + FUZZY_EPSILON);

	m_coneImpulseAxis = cross(twistAxis1, twistAxis2);
	if (m_coneImpulseAxis.length() > FUZZY_EPSILON)
		m_coneImpulseAxis = m_coneImpulseAxis.normalized();
	else
		m_coneImpulseAxis = c_axis[0];

	Scalar angularImpulseDenom1 = dot3(inertiaInv1 * m_coneImpulseAxis, m_coneImpulseAxis);
	Scalar angularImpulseDenom2 = dot3(inertiaInv2 * m_coneImpulseAxis, m_coneImpulseAxis);
	m_kCone = 1.0_simd / (angularImpulseDenom1 + angularImpulseDenom2);

	// Twist constraint.
	m_twistImpulseAxis = (twistAxis1 + twistAxis2) * 0.5_simd;

	angularImpulseDenom1 = dot3(inertiaInv1 * m_twistImpulseAxis, m_twistImpulseAxis);
	angularImpulseDenom2 = dot3(inertiaInv2 * m_twistImpulseAxis, m_twistImpulseAxis);
	m_kTwist = 1.0_simd / (angularImpulseDenom1 + angularImpulseDenom2);
}

void ConeTwistJointBullet::update(float deltaTime)
{
	const Scalar c_tau = 0.3_simd;
	const Scalar c_damping = 1.0_simd;

	const Transform tf1 = m_body1->getBodyTransform();
	const Transform tf2 = m_body2 ? m_body2->getBodyTransform() : Transform::identity();

	const Vector4 centerOfMass1 = tf1.translation().xyz1();
	const Vector4 centerOfMass2 = tf2.translation().xyz1();

	const Vector4 anchor1 = tf1 * m_anchor1;
	const Vector4 anchor2 = tf2 * m_anchor2;
	const Vector4 twistAxis1 = (tf1 * m_twistAxis1).normalized();
	const Vector4 coneAxis1 = (tf1 * m_coneAxis1).normalized();
	const Vector4 twistAxis2 = (tf2 * m_twistAxis2).normalized();
	const Vector4 coneAxis2 = (tf2 * m_coneAxis2).normalized();

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

	const Vector4 angularVelocity = angularVelocity2 - angularVelocity1;

	// Solve point constraint.
	const Scalar tauDt = c_tau / Scalar(deltaTime);
	for (int i = 0; i < 3; ++i)
	{
		const Scalar relativeVelocity = velocity2[i] - velocity1[i];	// Relative velocity on axis.
		const Scalar positionError = anchor2[i] - anchor1[i];			// Positional error on axis.

		const Scalar impulse = positionError * tauDt * m_jac[i].diagABInv + c_damping * relativeVelocity * m_jac[i].diagABInv;
		const Vector4 impulseVector = c_axis[i] * impulse;

		if (m_dynamicBody1)
			m_dynamicBody1->addImpulse(anchor1, impulseVector, false);
		if (m_dynamicBody2)
			m_dynamicBody2->addImpulse(anchor2, -impulseVector, false);
	}

	// Solve cone constraint.
	do
	{
		const Scalar phi = dot3(twistAxis1, twistAxis2);
		if (abs(phi) > 1.0f - std::numeric_limits< float >::epsilon())
			continue;

		const Scalar coneAngle = Scalar(acosf(phi));
		const Scalar coneAngleError = abs(coneAngle) - m_coneAngleLimit / 2.0_simd;
		if (coneAngleError <= 0.0_simd)
			continue;

		const Scalar amplitude = dot3(angularVelocity, m_coneImpulseAxis) * c_damping + coneAngleError * tauDt;
		const Scalar impulse = amplitude * m_kCone;

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
		const Vector4 coneAxis2in1 = Quaternion(twistAxis2, twistAxis1) * coneAxis2;
		const Vector4 coneAxis1ref = -cross(coneAxis1, twistAxis1).normalized();
		const Scalar a = dot3(coneAxis1, coneAxis2in1);
		const Scalar b = dot3(coneAxis1ref, coneAxis2in1);
		const Scalar twistAngle = Scalar(atan2f(b, a));

		Scalar twistAngleError = abs(twistAngle) - twistAngleLimit / 2.0_simd;
		if (twistAngleError <= 0.0f)
			continue;

		if (twistAngle < 0.0f)
			twistAngleError = -twistAngleError;

		const Scalar amplitude = dot3(angularVelocity, m_twistImpulseAxis) * c_damping + twistAngleError * tauDt;
		const Scalar impulse = amplitude * m_kTwist;

		if (m_dynamicBody1)
			m_dynamicBody1->addAngularImpulse(m_twistImpulseAxis * impulse, false);
		if (m_dynamicBody2)
			m_dynamicBody2->addAngularImpulse(m_twistImpulseAxis * -impulse, false);
	}
	while (false);
}

}

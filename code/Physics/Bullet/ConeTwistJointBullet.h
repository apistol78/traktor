/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_ConeTwistJointBullet_H
#define traktor_physics_ConeTwistJointBullet_H

#include "Core/Math/Transform.h"
#include "Physics/ConeTwistJoint.h"
#include "Physics/Bullet/JointBullet.h"
#include "Physics/Bullet/JointConstraint.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Body;
class ConeTwistJointDesc;

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS ConeTwistJointBullet : public JointBullet< ConeTwistJoint, JointConstraint >
{
	T_RTTI_CLASS;

public:
	ConeTwistJointBullet(IWorldCallback* callback, JointConstraint* constraint, const Transform& transform, BodyBullet* body1, const ConeTwistJointDesc* desc);

	ConeTwistJointBullet(IWorldCallback* callback, JointConstraint* constraint, const Transform& transform, BodyBullet* body1, BodyBullet* body2, const ConeTwistJointDesc* desc);

	virtual void prepare() T_OVERRIDE T_FINAL;

	virtual void update(float deltaTime) T_OVERRIDE T_FINAL;

private:
	struct Jacobian
	{
		Scalar diagABInv;
	};

	Ref< const ConeTwistJointDesc > m_desc;
	Vector4 m_anchor1;
	Vector4 m_anchor2;
	Vector4 m_coneAxis1;
	Vector4 m_coneAxis2;
	Vector4 m_twistAxis1;
	Vector4 m_twistAxis2;
	Ref< Body > m_dynamicBody1;
	Ref< Body > m_dynamicBody2;
	Jacobian m_jac[3];
	Scalar m_kCone;
	Scalar m_coneAngleLimit;
	Vector4 m_coneImpulseAxis;
	Scalar m_kTwist;
	Vector4 m_twistImpulseAxis;
};

	}
}

#endif	// traktor_physics_ConeTwistJointBullet_H

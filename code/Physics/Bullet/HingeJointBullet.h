/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_HingeJointBullet_H
#define traktor_physics_HingeJointBullet_H

#include "Physics/HingeJoint.h"
#include "Physics/Bullet/JointBullet.h"

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

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS HingeJointBullet : public JointBullet< HingeJoint, btHingeConstraint >
{
	T_RTTI_CLASS;

public:
	HingeJointBullet(IWorldCallback* callback, btHingeConstraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual Vector4 getAnchor() const T_OVERRIDE T_FINAL;

	virtual Vector4 getAxis() const T_OVERRIDE T_FINAL;

	virtual float getAngle() const T_OVERRIDE T_FINAL;

	virtual float getAngleVelocity() const T_OVERRIDE T_FINAL;

	virtual void setMotor(float targetVelocity, float maxImpulse) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_HingeJointBullet_H

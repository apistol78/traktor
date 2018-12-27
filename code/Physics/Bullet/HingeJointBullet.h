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

	virtual Vector4 getAnchor() const override final;

	virtual Vector4 getAxis() const override final;

	virtual float getAngle() const override final;

	virtual float getAngleVelocity() const override final;

	virtual void setMotor(float targetVelocity, float maxImpulse) override final;
};

	}
}

#endif	// traktor_physics_HingeJointBullet_H

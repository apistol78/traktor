/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_HingeJointHavok_H
#define traktor_physics_HingeJointHavok_H

#include "Physics/HingeJoint.h"
#include "Physics/Havok/JointHavok.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_HAVOK_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS HingeJointHavok : public JointHavok< HingeJoint >
{
	T_RTTI_CLASS;

public:
	HingeJointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2);

	virtual Vector4 getAnchor() const;

	virtual Vector4 getAxis() const;

	virtual float getAngle() const;

	virtual float getAngleVelocity() const;

	virtual void setMotor(float targetVelocity, float maxImpulse);
};

	}
}

#endif	// traktor_physics_HingeJointHavok_H

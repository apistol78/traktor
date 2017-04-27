/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_HingeJoint_H
#define traktor_physics_HingeJoint_H

#include "Physics/Joint.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Hinge joint.
 * \ingroup Physics
 */
class T_DLLCLASS HingeJoint : public Joint
{
	T_RTTI_CLASS;

public:
	virtual Vector4 getAnchor() const = 0;

	virtual Vector4 getAxis() const = 0;

	virtual float getAngle() const = 0;

	virtual float getAngleVelocity() const = 0;

	virtual void setMotor(float targetVelocity, float maxImpulse) = 0;
};

	}
}

#endif	// traktor_physics_HingeJoint_H

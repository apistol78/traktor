#ifndef traktor_physics_HingeJointPhysX_H
#define traktor_physics_HingeJointPhysX_H

#include "Physics/HingeJoint.h"
#include "Physics/PhysX/JointPhysX.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS HingeJointPhysX : public JointPhysX< HingeJoint >
{
	T_RTTI_CLASS;

public:
	HingeJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2);

	virtual Vector4 getAnchor() const;

	virtual Vector4 getAxis() const;

	virtual float getAngle() const;

	virtual float getAngleVelocity() const;

	virtual void setMotor(float targetVelocity, float maxImpulse);
};

	}
}

#endif	// traktor_physics_HingeJointPhysX_H

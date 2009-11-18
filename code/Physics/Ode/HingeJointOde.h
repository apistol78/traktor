#ifndef traktor_physics_HingeJointOde_H
#define traktor_physics_HingeJointOde_H

#include <ode/ode.h>
#include "Physics/HingeJoint.h"
#include "Physics/Ode/JointOde.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_ODE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup ODE
 */
class T_DLLCLASS HingeJointOde : public JointOde< HingeJoint >
{
	T_RTTI_CLASS;

public:
	HingeJointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2);

	virtual Vector4 getAnchor() const;

	virtual Vector4 getAxis() const;

	virtual float getAngle() const;

	virtual float getAngleVelocity() const;
};

	}
}

#endif	// traktor_physics_HingeJointOde_H

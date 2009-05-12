#ifndef traktor_physics_BallJointOde_H
#define traktor_physics_BallJointOde_H

#include <ode/ode.h>
#include "Physics/BallJoint.h"
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
class T_DLLCLASS BallJointOde : public JointOde< BallJoint >
{
	T_RTTI_CLASS(BallJointOde)

public:
	BallJointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2);

	virtual Vector4 getAnchor() const;
};

	}
}

#endif	// traktor_physics_BallJointOde_H

#ifndef traktor_physics_Hinge2JointOde_H
#define traktor_physics_Hinge2JointOde_H

#include <ode/ode.h>
#include "Physics/Hinge2Joint.h"
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
class T_DLLCLASS Hinge2JointOde : public JointOde< Hinge2Joint >
{
	T_RTTI_CLASS(Hinge2JointOde)

public:
	Hinge2JointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2);

	virtual void addTorques(float torqueAxis1, float torqueAxis2);

	virtual float getAngleAxis1() const;

	virtual void setVelocityAxis1(float velocityAxis1);

	virtual void setVelocityAxis2(float velocityAxis2);

	virtual void getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const;
};

	}
}

#endif	// traktor_physics_Hinge2JointOde_H

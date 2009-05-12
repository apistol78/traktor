#ifndef traktor_physics_ConeTwistJointOde_H
#define traktor_physics_ConeTwistJointOde_H

#include <ode/ode.h>
#include "Core/Heap/Ref.h"
#include "Physics/ConeTwistJoint.h"
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

class ConeTwistJointDesc;

/*!
 * \ingroup ODE
 */
class T_DLLCLASS ConeTwistJointOde : public JointOde< ConeTwistJoint >
{
	T_RTTI_CLASS(ConeTwistJointOde)

public:
	ConeTwistJointOde(DestroyCallback* callback, Body* body1, Body* body2, const ConeTwistJointDesc* desc);

	virtual Vector4 getAnchor() const;

	void update(float deltaTime);

private:
	Ref< const ConeTwistJointDesc > m_desc;
	Vector4 m_anchor1;
	Vector4 m_anchor2;
	Vector4 m_coneAxis1;
	Vector4 m_coneAxis2;
	Vector4 m_twistAxis1;
	Vector4 m_twistAxis2;
};

	}
}

#endif	// traktor_physics_ConeTwistJointOde_H

#ifndef traktor_physics_BallJointPhysX_H
#define traktor_physics_BallJointPhysX_H

#include "Core/Heap/Ref.h"
#include "Physics/BallJoint.h"
#include "Physics/PhysX/JointPhysX.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS BallJointPhysX : public JointPhysX< BallJoint >
{
	T_RTTI_CLASS(BallJointPhysX)

public:
	BallJointPhysX(DestroyCallbackPhysX* callback, NxJoint* joint, Body* body1, Body* body2);

	virtual Vector4 getAnchor() const;
};

	}
}

#endif	// traktor_physics_BallJointPhysX_H

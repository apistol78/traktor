/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_BallJointPhysX_H
#define traktor_physics_BallJointPhysX_H

#include "Physics/BallJoint.h"
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
class T_DLLCLASS BallJointPhysX : public JointPhysX< BallJoint >
{
	T_RTTI_CLASS;

public:
	BallJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2);

	virtual void setAnchor(const Vector4& anchor);

	virtual Vector4 getAnchor() const;
};

	}
}

#endif	// traktor_physics_BallJointPhysX_H

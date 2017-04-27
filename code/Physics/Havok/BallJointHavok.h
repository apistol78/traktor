/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_BallJointHavok_H
#define traktor_physics_BallJointHavok_H

#include "Physics/BallJoint.h"
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
 * \ingroup Havok
 */
class T_DLLCLASS BallJointHavok : public JointHavok< BallJoint >
{
	T_RTTI_CLASS;

public:
	BallJointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2);

	virtual void setAnchor(const Vector4& anchor);

	virtual Vector4 getAnchor() const;
};

	}
}

#endif	// traktor_physics_BallJointHavok_H

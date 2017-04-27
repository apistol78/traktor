/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_BallJoint_H
#define traktor_physics_BallJoint_H

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

/*! \brief Ball joint.
 * \ingroup Physics
 */
class T_DLLCLASS BallJoint : public Joint
{
	T_RTTI_CLASS;

public:
	virtual void setAnchor(const Vector4& anchor) = 0;

	virtual Vector4 getAnchor() const = 0;
};

	}
}

#endif	// traktor_physics_BallJoint_H

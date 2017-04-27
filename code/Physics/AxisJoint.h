/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_AxisJoint_H
#define traktor_physics_AxisJoint_H

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

/*! \brief Axis joint.
 * \ingroup Physics
 */
class T_DLLCLASS AxisJoint : public Joint
{
	T_RTTI_CLASS;

public:
	virtual Vector4 getAxis() const = 0;
};

	}
}

#endif	// traktor_physics_AxisJoint_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_FixedJoint_H
#define traktor_physics_FixedJoint_H

#include "Physics/Joint.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Fixed joint.
 * \ingroup Physics
 */
class T_DLLCLASS FixedJoint : public Joint
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_physics_FixedJoint_H

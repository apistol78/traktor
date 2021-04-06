#pragma once

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

/*! Dof joint.
 * \ingroup Physics
 */
class T_DLLCLASS DofJoint : public Joint
{
	T_RTTI_CLASS;
};

	}
}


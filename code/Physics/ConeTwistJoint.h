#pragma once

#include "Core/Math/Vector4.h"
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

/*! Cone twist joint.
 * \ingroup Physics
 */
class T_DLLCLASS ConeTwistJoint : public Joint
{
	T_RTTI_CLASS;
};

	}
}


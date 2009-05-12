#ifndef traktor_physics_ConeTwistJoint_H
#define traktor_physics_ConeTwistJoint_H

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

/*! \brief Cone twist joint.
 * \ingroup Physics
 */
class T_DLLCLASS ConeTwistJoint : public Joint
{
	T_RTTI_CLASS(ConeTwistJoint)
};

	}
}

#endif	// traktor_physics_ConeTwistJoint_H

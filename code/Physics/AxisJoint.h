#pragma once

#include "Physics/Joint.h"
#include "Core/Math/Vector4.h"

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

/*! Axis joint.
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


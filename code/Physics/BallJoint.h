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

/*! Ball joint.
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


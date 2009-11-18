#ifndef traktor_physics_StaticBody_H
#define traktor_physics_StaticBody_H

#include "Physics/Body.h"

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

/*! \brief Static rigid body.
 * \ingroup Physics
 */
class T_DLLCLASS StaticBody : public Body
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_physics_StaticBody_H

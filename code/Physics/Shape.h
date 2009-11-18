#ifndef traktor_physics_Shape_H
#define traktor_physics_Shape_H

#include "Core/Object.h"

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

/*! \brief Collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS Shape : public Object
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_physics_Shape_H

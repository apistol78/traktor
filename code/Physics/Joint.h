#ifndef traktor_physics_Joint_H
#define traktor_physics_Joint_H

#include "Core/Object.h"

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

class Body;

/*! \brief Joint base class.
 * \ingroup Physics
 */
class T_DLLCLASS Joint : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! \brief Get first constraint body. */
	virtual Body* getBody1() = 0;

	/*! \brief Get second constraint body.
	 *
	 * \note
	 * This return null if first body is constrained
	 * to world.
	 */
	virtual Body* getBody2() = 0;

	/*! \brief
	 */
	virtual void* getInternal() = 0;
};

	}
}

#endif	// traktor_physics_Joint_H

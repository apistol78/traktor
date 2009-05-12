#ifndef traktor_physics_ode_Types_H
#define traktor_physics_ode_Types_H

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup ODE
 */
struct DestroyCallback
{
	virtual void bodyDestroyed(class Body* body) = 0;

	virtual void jointDestroyed(class Joint* joint) = 0;
};

	}
}

#endif	// traktor_physics_ode_Types_H

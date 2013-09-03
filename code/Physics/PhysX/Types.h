#ifndef traktor_physics_physx_Types_H
#define traktor_physics_physx_Types_H

namespace physx
{

class PxRigidActor;
class PxJoint;

}

namespace traktor
{
	namespace physics
	{

class Body;
class Joint;

/*!
 * \ingroup PhysX
 */
struct DestroyCallbackPhysX
{
	virtual void destroyBody(Body* owner, physx::PxRigidActor* actor) = 0;

	virtual void destroyJoint(Joint* owner, physx::PxJoint* joint) = 0;
};

	}
}

#endif	// traktor_physics_physx_Types_H

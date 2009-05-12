#ifndef traktor_physics_physx_Types_H
#define traktor_physics_physx_Types_H

class NxActor;
class NxJoint;

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
	virtual void destroyBody(Body* owner, NxActor& actor) = 0;

	virtual void destroyJoint(Joint* owner, NxJoint& joint) = 0;
};

	}
}

#endif	// traktor_physics_physx_Types_H

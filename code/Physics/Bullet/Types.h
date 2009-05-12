#ifndef traktor_physics_bullet_Types_H
#define traktor_physics_bullet_Types_H

class btRigidBody;
class btCollisionShape;
class btTypedConstraint;

namespace traktor
{
	namespace physics
	{

class Body;
class Joint;

/*!
 * \ingroup Bullet
 */
struct DestroyCallback
{
	virtual void destroyBody(Body* body, btRigidBody* rigidBody, btCollisionShape* shape) = 0;

	virtual void destroyJoint(Joint* joint, btTypedConstraint* constraint) = 0;
};

	}
}

#endif	// traktor_physics_bullet_Types_H

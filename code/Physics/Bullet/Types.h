/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_bullet_Types_H
#define traktor_physics_bullet_Types_H

class btRigidBody;
class btCollisionShape;
class btTypedConstraint;

namespace traktor
{
	namespace physics
	{

class BodyBullet;
class Joint;

/*!
 * \ingroup Bullet
 */
struct IWorldCallback
{
	virtual ~IWorldCallback() {}

	virtual void insertBody(btRigidBody* rigidBody, uint16_t collisionGroup, uint16_t collisionFilter) = 0;

	virtual void removeBody(btRigidBody* rigidBody) = 0;

	virtual void insertConstraint(btTypedConstraint* constraint) = 0;

	virtual void removeConstraint(btTypedConstraint* constraint) = 0;

	virtual void destroyBody(BodyBullet* body, btRigidBody* rigidBody, btCollisionShape* shape) = 0;

	virtual void destroyConstraint(Joint* joint, btTypedConstraint* constraint) = 0;
};

	}
}

#endif	// traktor_physics_bullet_Types_H

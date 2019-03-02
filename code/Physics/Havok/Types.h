#pragma once

#include "Physics/Havok/HvkRef.h"

class hkpRigidBody;
class hkpConstraintInstance;

namespace traktor
{
	namespace physics
	{

class Body;
class Joint;

/*!
 * \ingroup Havok
 */
struct DestroyCallbackHavok
{
	virtual void destroyBody(Body* body, const HvkRef< hkpRigidBody >& rigidBody) = 0;

	virtual void destroyJoint(Joint* joint, const HvkRef< hkpConstraintInstance >& constraint) = 0;
};

	}
}


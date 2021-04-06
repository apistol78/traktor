#pragma once

#include "Physics/DofJoint.h"
#include "Physics/Bullet/JointBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS DofJointBullet : public JointBullet< DofJoint, btGeneric6DofConstraint >
{
	T_RTTI_CLASS;

public:
	DofJointBullet(IWorldCallback* callback, btGeneric6DofConstraint* constraint, BodyBullet* body1, BodyBullet* body2);
};

	}
}


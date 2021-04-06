#include "Physics/Bullet/DofJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DofJointBullet", DofJointBullet, DofJoint)

DofJointBullet::DofJointBullet(IWorldCallback* callback, btGeneric6DofConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< DofJoint, btGeneric6DofConstraint >(callback, constraint, body1, body2)
{
}

	}
}

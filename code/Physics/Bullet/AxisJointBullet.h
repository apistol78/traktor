#ifndef traktor_physics_AxisJointBullet_H
#define traktor_physics_AxisJointBullet_H

#include "Physics/AxisJoint.h"
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
class T_DLLCLASS AxisJointBullet : public JointBullet< AxisJoint, btHingeConstraint >
{
	T_RTTI_CLASS;

public:
	AxisJointBullet(IWorldCallback* callback, btHingeConstraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual Vector4 getAxis() const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_AxisJointBullet_H

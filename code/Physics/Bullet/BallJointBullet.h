#ifndef traktor_physics_BallJointBullet_H
#define traktor_physics_BallJointBullet_H

#include "Physics/BallJoint.h"
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
class T_DLLCLASS BallJointBullet : public JointBullet< BallJoint, btPoint2PointConstraint >
{
	T_RTTI_CLASS;

public:
	BallJointBullet(IWorldCallback* callback, btPoint2PointConstraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual void setAnchor(const Vector4& anchor) T_OVERRIDE T_FINAL;

	virtual Vector4 getAnchor() const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_BallJointBullet_H

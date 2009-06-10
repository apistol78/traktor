#ifndef traktor_physics_BallJointBullet_H
#define traktor_physics_BallJointBullet_H

#include "Core/Heap/Ref.h"
#include "Physics/BallJoint.h"
#include "Physics/Bullet/JointBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(BallJointBullet)

public:
	BallJointBullet(DestroyCallback* callback, btPoint2PointConstraint* constraint, Body* body1, Body* body2);

	virtual void setAnchor(const Vector4& anchor);

	virtual Vector4 getAnchor() const;
};

	}
}

#endif	// traktor_physics_BallJointBullet_H

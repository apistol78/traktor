#ifndef traktor_physics_Hinge2JointBullet_H
#define traktor_physics_Hinge2JointBullet_H

#include "Physics/Hinge2Joint.h"
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
class T_DLLCLASS Hinge2JointBullet : public JointBullet< Hinge2Joint, btHinge2Constraint >
{
	T_RTTI_CLASS;

public:
	Hinge2JointBullet(IWorldCallback* callback, btHinge2Constraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual void addTorques(float torqueAxis1, float torqueAxis2);

	virtual float getAngleAxis1() const;

	virtual void setVelocityAxis1(float velocityAxis1);

	virtual void setVelocityAxis2(float velocityAxis2);

	virtual void getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const;
};

	}
}

#endif	// traktor_physics_Hinge2JointBullet_H

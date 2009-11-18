#ifndef traktor_physics_StaticBodyBullet_H
#define traktor_physics_StaticBodyBullet_H

#include "Physics/StaticBody.h"
#include "Physics/Bullet/BodyBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

// Bullet forward declarations.
class btRigidBody;
class btCollisionShape;

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS StaticBodyBullet : public BodyBullet< StaticBody >
{
	T_RTTI_CLASS;

public:
	StaticBodyBullet(DestroyCallback* callback, btDynamicsWorld* dynamicsWorld, btRigidBody* body, btCollisionShape* shape);

	virtual void setTransform(const Transform& transform);

	virtual Transform getTransform() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;

private:
	bool m_enable;
};

	}
}

#endif	// traktor_physics_StaticBodyBullet_H

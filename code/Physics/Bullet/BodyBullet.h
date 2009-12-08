#ifndef traktor_physics_BodyBullet_H
#define traktor_physics_BodyBullet_H

#include <btBulletDynamicsCommon.h>
#include "Core/Thread/Atomic.h"
#include "Physics/Bullet/Types.h"

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
template < typename Outer >
class BodyBullet : public Outer
{
public:
	BodyBullet(DestroyCallback* callback, btDynamicsWorld* dynamicsWorld, btRigidBody* body, btCollisionShape* shape)
	:	m_callback(callback)
	,	m_dynamicsWorld(dynamicsWorld)
	,	m_body(body)
	,	m_shape(shape)
	{
	}

	virtual ~BodyBullet()
	{
		destroy();
	}

	virtual void destroy()
	{
		DestroyCallback* callback = Atomic::exchange< DestroyCallback* >(m_callback, 0);
		if (callback)
		{
			callback->destroyBody(this, m_body, m_shape);
			m_body = 0;
			m_shape = 0;
		}
	}

	inline btDynamicsWorld* getBtDynamicsWorld() const { return m_dynamicsWorld; }

	inline btRigidBody* getBtRigidBody() const { return m_body; }

	inline btCollisionShape* getBtCollisionShape() const { return m_shape; }

protected:
	DestroyCallback* m_callback;
	btDynamicsWorld* m_dynamicsWorld;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
};

	}
}

#endif	// traktor_physics_BodyBullet_H

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
	BodyBullet(
		DestroyCallback* callback,
		btDynamicsWorld* dynamicsWorld,
		btRigidBody* body,
		btCollisionShape* shape,
		uint32_t group
	)
	:	m_callback(callback)
	,	m_dynamicsWorld(dynamicsWorld)
	,	m_body(body)
	,	m_shape(shape)
	,	m_group(group)
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

	btDynamicsWorld* getBtDynamicsWorld() const { return m_dynamicsWorld; }

	btRigidBody* getBtRigidBody() const { return m_body; }

	btCollisionShape* getBtCollisionShape() const { return m_shape; }

	uint32_t getGroup() const { return m_group; }

protected:
	DestroyCallback* m_callback;
	btDynamicsWorld* m_dynamicsWorld;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
	uint32_t m_group;
};

	}
}

#endif	// traktor_physics_BodyBullet_H

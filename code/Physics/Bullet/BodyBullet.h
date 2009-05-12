#ifndef traktor_physics_BodyBullet_H
#define traktor_physics_BodyBullet_H

#include <btBulletDynamicsCommon.h>
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
	BodyBullet(DestroyCallback* callback, btRigidBody* body, btCollisionShape* shape)
	:	m_callback(callback)
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
		if (m_callback)
		{
			m_callback->destroyBody(this, m_body, m_shape);
			m_callback = 0;
		}

		m_body = 0;
		m_shape = 0;
	}

	inline btRigidBody* getBtRigidBody() const { return m_body; }

	inline btCollisionShape* getBtCollisionShape() const { return m_shape; }

protected:
	DestroyCallback* m_callback;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
};

	}
}

#endif	// traktor_physics_BodyBullet_H

#include <btBulletDynamicsCommon.h>
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/StaticBodyBullet.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.StaticBodyBullet", StaticBodyBullet, StaticBody)

StaticBodyBullet::StaticBodyBullet(
	IWorldCallback* callback,
	btDynamicsWorld* dynamicsWorld,
	btRigidBody* body,
	btCollisionShape* shape,
	uint32_t collisionGroup,
	uint32_t collisionMask
)
:	BodyBullet< StaticBody >(callback, dynamicsWorld, body, shape, collisionGroup, collisionMask)
{
}

void StaticBodyBullet::setTransform(const Transform& transform)
{
	T_ASSERT (m_body);
	
	btTransform bt = toBtTransform(transform);
	m_body->setWorldTransform(bt);

	// Update motion state's transform as well in case if kinematic body.
	if (m_body->isKinematicObject())
		m_body->getMotionState()->setWorldTransform(bt);
}

Transform StaticBodyBullet::getTransform() const
{
	T_ASSERT (m_body);
	return fromBtTransform(m_body->getWorldTransform());
}

void StaticBodyBullet::setActive(bool active)
{
	if (!m_body->isKinematicObject())
		m_body->setActivationState(active ? ACTIVE_TAG : ISLAND_SLEEPING);
}

bool StaticBodyBullet::isActive() const
{
	return m_body->isActive();
}

	}
}

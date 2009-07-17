#include <btBulletDynamicsCommon.h>
#include "Physics/Bullet/StaticBodyBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.StaticBodyBullet", StaticBodyBullet, StaticBody)

StaticBodyBullet::StaticBodyBullet(DestroyCallback* callback, btDynamicsWorld* dynamicsWorld, btRigidBody* body, btCollisionShape* shape)
:	BodyBullet< StaticBody >(callback, dynamicsWorld, body, shape)
,	m_enable(false)
{
}

void StaticBodyBullet::setTransform(const Matrix44& transform)
{
	T_ASSERT (m_body);
	
	btTransform bt = toBtTransform(transform);
	m_body->setWorldTransform(bt);

	// Update motion state's transform as well in case if kinematic body.
	if (m_body->isKinematicObject())
		m_body->getMotionState()->setWorldTransform(bt);
}

Matrix44 StaticBodyBullet::getTransform() const
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

void StaticBodyBullet::setEnable(bool enable)
{
	if (enable == m_enable)
		return;

	if (enable)
	{
		//m_dynamicsWorld->addCollisionObject(m_body);
		m_dynamicsWorld->addRigidBody(m_body);
	}
	else
	{
		//m_dynamicsWorld->removeCollisionObject(m_body);
		m_dynamicsWorld->removeRigidBody(m_body);
	}

	m_enable = enable;
}

bool StaticBodyBullet::isEnable() const
{
	return m_enable;
}

	}
}

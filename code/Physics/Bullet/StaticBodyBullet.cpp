#include <btBulletDynamicsCommon.h>
#include "Physics/Bullet/StaticBodyBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.StaticBodyBullet", StaticBodyBullet, StaticBody)

StaticBodyBullet::StaticBodyBullet(DestroyCallback* callback, btRigidBody* body, btCollisionShape* shape)
:	BodyBullet< StaticBody >(callback, body, shape)
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

void StaticBodyBullet::setEnable(bool enable)
{
	if (!m_body->isKinematicObject())
		m_body->setActivationState(enable ? ACTIVE_TAG : ISLAND_SLEEPING);
}

bool StaticBodyBullet::getEnable() const
{
	return m_body->isActive();
}

	}
}

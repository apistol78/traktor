#include <btBulletDynamicsCommon.h>
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Physics/DynamicBodyState.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/DynamicBodyBullet.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

inline Vector4 convert(const DynamicBodyBullet* body, const Vector4& v, bool localSpace)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan((v).w()));
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DynamicBodyBullet", DynamicBodyBullet, DynamicBody)

DynamicBodyBullet::DynamicBodyBullet(
	IWorldCallback* callback,
	btDynamicsWorld* dynamicsWorld,
	btRigidBody* body,
	btCollisionShape* shape,
	uint32_t collisionGroup,
	uint32_t collisionMask
)
:	BodyBullet< DynamicBody >(callback, dynamicsWorld, body, shape, collisionGroup, collisionMask)
{
}

void DynamicBodyBullet::setTransform(const Transform& transform)
{
	T_ASSERT (m_body);
	m_body->setWorldTransform(toBtTransform(transform));
}

Transform DynamicBodyBullet::getTransform() const
{
	T_ASSERT (m_body);
	return fromBtTransform(m_body->getWorldTransform());
}

void DynamicBodyBullet::reset()
{
	T_ASSERT (m_body);
	m_body->clearForces();
}

void DynamicBodyBullet::setMass(float mass, const Vector4& inertiaTensor)
{
	T_ASSERT (m_body);
	m_body->setMassProps(mass, toBtVector3(inertiaTensor));
}

float DynamicBodyBullet::getInverseMass() const
{
	T_ASSERT (m_body);
	return m_body->getInvMass();
}

Matrix33 DynamicBodyBullet::getInertiaTensorInverseWorld() const
{
	T_ASSERT (m_body);
	return fromBtMatrix(m_body->getInvInertiaTensorWorld());
}

void DynamicBodyBullet::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	T_ASSERT (m_body);

	Vector4 at_ = convert(this, at, localSpace);
	Vector4 force_ = convert(this, force, localSpace);
	Vector4 relativeAt = at_ - fromBtVector3(m_body->getCenterOfMassPosition(), 1.0f);

	m_body->applyForce(
		toBtVector3(force_),
		toBtVector3(relativeAt)
	);
}

void DynamicBodyBullet::addTorque(const Vector4& torque, bool localSpace)
{
	T_ASSERT (m_body);
	Vector4 torque_ = convert(this, torque, localSpace);
	m_body->applyTorque(toBtVector3(torque_));
}

void DynamicBodyBullet::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	T_ASSERT (m_body);
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	m_body->applyCentralImpulse(toBtVector3(linearImpulse_));
}

void DynamicBodyBullet::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	T_ASSERT (m_body);
	Vector4 angularImpulse_ = convert(this, angularImpulse, localSpace);
	m_body->applyTorqueImpulse(toBtVector3(angularImpulse_));
}

void DynamicBodyBullet::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	T_ASSERT (m_body);

	Vector4 at_ = convert(this, at, localSpace);
	Vector4 impulse_ = convert(this, impulse, localSpace);
	Vector4 relativeAt = at_ - fromBtVector3(m_body->getCenterOfMassPosition(), 1.0f);

	m_body->applyImpulse(
		toBtVector3(impulse_),
		toBtVector3(relativeAt)
	);
}

void DynamicBodyBullet::setLinearVelocity(const Vector4& linearVelocity)
{
	T_ASSERT (m_body);
	m_body->setLinearVelocity(toBtVector3(linearVelocity));
}

Vector4 DynamicBodyBullet::getLinearVelocity() const
{
	T_ASSERT (m_body);
	return fromBtVector3(m_body->getLinearVelocity(), 0.0f);
}

void DynamicBodyBullet::setAngularVelocity(const Vector4& angularVelocity)
{
	T_ASSERT (m_body);
	m_body->setAngularVelocity(toBtVector3(angularVelocity));
}

Vector4 DynamicBodyBullet::getAngularVelocity() const
{
	T_ASSERT (m_body);
	return fromBtVector3(m_body->getAngularVelocity(), 0.0f);
}

Vector4 DynamicBodyBullet::getVelocityAt(const Vector4& at, bool localSpace) const
{
	T_ASSERT (m_body);

	btVector3 relPos;
	if (localSpace)
		relPos = toBtVector3(getTransform() * at - getTransform().translation());
	else
		relPos = toBtVector3(at - getTransform().translation());

	return fromBtVector3(
		m_body->getVelocityInLocalPoint(relPos),
		0.0f
	);
}

bool DynamicBodyBullet::setState(const DynamicBodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
	return true;
}

DynamicBodyState DynamicBodyBullet::getState() const
{
	DynamicBodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void DynamicBodyBullet::setActive(bool active)
{
	m_body->setActivationState(active ? ACTIVE_TAG : ISLAND_SLEEPING);
}

bool DynamicBodyBullet::isActive() const
{
	return m_body->isActive();
}

	}
}

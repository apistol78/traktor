#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include "Core/Math/Float.h"
#include "Physics/Havok/BodyHavok.h"
#include "Physics/Havok/Conversion.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

T_FORCE_INLINE Vector4 convert(const BodyHavok* body, const Vector4& v, bool localSpace)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan((v).w()));
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyHavok", BodyHavok, Body)

BodyHavok::BodyHavok(DestroyCallbackHavok* callback, const HvkRef< hkpRigidBody >& rigidBody, float simulationDeltaTime)
:	m_callback(callback)
,	m_rigidBody(rigidBody)
,	m_simulationDeltaTime(simulationDeltaTime)
{
}

void BodyHavok::destroy()
{
	if (m_callback)
	{
		m_callback->destroyBody(this, m_rigidBody);
		m_callback = 0;
	}

	m_rigidBody.release();

	Body::destroy();
}

void BodyHavok::setTransform(const Transform& transform)
{
	m_rigidBody->setTransform(toHkTransform(transform));
}

Transform BodyHavok::getTransform() const
{
	return fromHkTransform(m_rigidBody->getTransform());
}

Transform BodyHavok::getCenterTransform() const
{
	return getTransform();
}

bool BodyHavok::isStatic() const
{
	return false;
}

bool BodyHavok::isKinematic() const
{
	return false;
}

void BodyHavok::setActive(bool active)
{
	if (active)
		m_rigidBody->activate();
	else
		m_rigidBody->deactivate();
}

bool BodyHavok::isActive() const
{
	return m_rigidBody->isActive();
}

void BodyHavok::setEnable(bool enable)
{
}

bool BodyHavok::isEnable() const
{
	return true;
}

void BodyHavok::reset()
{
	setLinearVelocity(Vector4::zero());
	setAngularVelocity(Vector4::zero());
}

void BodyHavok::setMass(float mass, const Vector4& inertiaTensor)
{
	hkMatrix3 m;
	m.setDiagonal(
		inertiaTensor.x(),
		inertiaTensor.y(),
		inertiaTensor.z()
	);

	m_rigidBody->setMass(mass);
	m_rigidBody->setInertiaLocal(m);
}

float BodyHavok::getInverseMass() const
{
	return m_rigidBody->getMassInv();
}

Matrix33 BodyHavok::getInertiaTensorInverseWorld() const
{
	hkMatrix3 invInertia;
	m_rigidBody->getInertiaInvWorld(invInertia);
	return fromHkMatrix3(invInertia);
}

void BodyHavok::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 force_ = convert(this, force, localSpace);

	m_rigidBody->applyForce(
		m_simulationDeltaTime,
		toHkVector4(force_),
		toHkVector4(at_)
	);
}

void BodyHavok::addTorque(const Vector4& torque, bool localSpace)
{
	Vector4 torque_ = convert(this, torque, localSpace);

	m_rigidBody->applyTorque(
		m_simulationDeltaTime,
		toHkVector4(torque_)
	);
}

void BodyHavok::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	m_rigidBody->applyLinearImpulse(toHkVector4(linearImpulse_));
}

void BodyHavok::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	Vector4 angularImpulse_ = convert(this, angularImpulse_, localSpace);
	m_rigidBody->applyAngularImpulse(toHkVector4(angularImpulse_));
}

void BodyHavok::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 impulse_ = convert(this, at, localSpace);
	m_rigidBody->applyPointImpulse(
		toHkVector4(impulse_),
		toHkVector4(at_)
	);
}

void BodyHavok::setLinearVelocity(const Vector4& linearVelocity)
{
	m_rigidBody->setLinearVelocity(toHkVector4(linearVelocity));
}

Vector4 BodyHavok::getLinearVelocity() const
{
	return fromHkVector4(m_rigidBody->getLinearVelocity());
}

void BodyHavok::setAngularVelocity(const Vector4& angularVelocity)
{
	m_rigidBody->setAngularVelocity(toHkVector4(angularVelocity));
}

Vector4 BodyHavok::getAngularVelocity() const
{
	return fromHkVector4(m_rigidBody->getAngularVelocity());
}

Vector4 BodyHavok::getVelocityAt(const Vector4& at, bool localSpace) const
{
	Vector4 at_ = convert(this, at, localSpace);

	hkVector4 velocity;
	m_rigidBody->getPointVelocity(toHkVector4(at_), velocity);

	return fromHkVector4(velocity);
}

bool BodyHavok::solveStateConstraint(const BodyState& state)
{
	return false;
}

bool BodyHavok::setState(const BodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
	return true;
}

BodyState BodyHavok::getState() const
{
	BodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void BodyHavok::integrate(float deltaTime)
{
}

	}
}

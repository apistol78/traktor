#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include "Physics/Havok/DynamicBodyHavok.h"
#include "Physics/Havok/Conversion.h"
#include "Core/Math/Float.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

T_FORCE_INLINE Vector4 convert(const DynamicBodyHavok* body, const Vector4& v, bool localSpace)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan((v).w()));
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DynamicBodyHavok", DynamicBodyHavok, DynamicBody)

DynamicBodyHavok::DynamicBodyHavok(DestroyCallbackHavok* callback, const HvkRef< hkpRigidBody >& rigidBody, float simulationDeltaTime)
:	BodyHavok< DynamicBody >(callback, rigidBody)
,	m_simulationDeltaTime(simulationDeltaTime)
{
}

void DynamicBodyHavok::setTransform(const Transform& transform)
{
	m_rigidBody->setTransform(toHkTransform(transform));
}

Transform DynamicBodyHavok::getTransform() const
{
	return fromHkTransform(m_rigidBody->getTransform());
}

void DynamicBodyHavok::reset()
{
	setLinearVelocity(Vector4::zero());
	setAngularVelocity(Vector4::zero());
}

void DynamicBodyHavok::setMass(float mass, const Vector4& inertiaTensor)
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

float DynamicBodyHavok::getInverseMass() const
{
	return m_rigidBody->getMassInv();
}

Matrix33 DynamicBodyHavok::getInertiaTensorInverseWorld() const
{
	hkMatrix3 invInertia;
	m_rigidBody->getInertiaInvWorld(invInertia);
	return fromHkMatrix3(invInertia);
}

void DynamicBodyHavok::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 force_ = convert(this, force, localSpace);

	m_rigidBody->applyForce(
		m_simulationDeltaTime,
		toHkVector4(force_),
		toHkVector4(at_)
	);
}

void DynamicBodyHavok::addTorque(const Vector4& torque, bool localSpace)
{
	Vector4 torque_ = convert(this, torque, localSpace);

	m_rigidBody->applyTorque(
		m_simulationDeltaTime,
		toHkVector4(torque_)
	);
}

void DynamicBodyHavok::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	m_rigidBody->applyLinearImpulse(toHkVector4(linearImpulse_));
}

void DynamicBodyHavok::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	Vector4 angularImpulse_ = convert(this, angularImpulse_, localSpace);
	m_rigidBody->applyAngularImpulse(toHkVector4(angularImpulse_));
}

void DynamicBodyHavok::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 impulse_ = convert(this, at, localSpace);
	m_rigidBody->applyPointImpulse(
		toHkVector4(impulse_),
		toHkVector4(at_)
	);
}

void DynamicBodyHavok::setLinearVelocity(const Vector4& linearVelocity)
{
	m_rigidBody->setLinearVelocity(toHkVector4(linearVelocity));
}

Vector4 DynamicBodyHavok::getLinearVelocity() const
{
	return fromHkVector4(m_rigidBody->getLinearVelocity());
}

void DynamicBodyHavok::setAngularVelocity(const Vector4& angularVelocity)
{
	m_rigidBody->setAngularVelocity(toHkVector4(angularVelocity));
}

Vector4 DynamicBodyHavok::getAngularVelocity() const
{
	return fromHkVector4(m_rigidBody->getAngularVelocity());
}

Vector4 DynamicBodyHavok::getVelocityAt(const Vector4& at, bool localSpace) const
{
	Vector4 at_ = convert(this, at, localSpace);

	hkVector4 velocity;
	m_rigidBody->getPointVelocity(toHkVector4(at_), velocity);

	return fromHkVector4(velocity);
}

bool DynamicBodyHavok::setState(const DynamicBodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
	return true;
}

DynamicBodyState DynamicBodyHavok::getState() const
{
	DynamicBodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void DynamicBodyHavok::setActive(bool active)
{
	if (active)
		m_rigidBody->activate();
	else
		m_rigidBody->deactivate();
}

bool DynamicBodyHavok::isActive() const
{
	return m_rigidBody->isActive();
}

void DynamicBodyHavok::setEnable(bool enable)
{
}

bool DynamicBodyHavok::isEnable() const
{
	return true;
}

	}
}

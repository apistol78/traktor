#include <NxPhysics.h>
#include "Core/Math/Float.h"
#include "Physics/PhysX/BodyPhysX.h"
#include "Physics/PhysX/Conversion.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

inline Vector4 convert(const BodyPhysX* body, const Vector4& v, bool localSpace)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan((v).w()));
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyPhysX", BodyPhysX, Body)

BodyPhysX::BodyPhysX(DestroyCallbackPhysX* callback, NxActor* actor)
:	m_callback(callback)
,	m_actor(actor)
{
}

void BodyPhysX::destroy()
{
	if (m_callback)
	{
		m_callback->destroyBody(this, *m_actor);
		m_callback = 0;
	}
	m_actor = 0;
	Body::destroy();
}

void BodyPhysX::setTransform(const Transform& transform)
{
	m_actor->setGlobalPose(toNxMat34(transform));
}

Transform BodyPhysX::getTransform() const
{
	return fromNxMat34(m_actor->getGlobalPose());
}

Transform BodyPhysX::getCenterTransform() const
{
	return getTransform();
}

bool BodyPhysX::isStatic() const
{
	return !m_actor->isDynamic();
}

void BodyPhysX::setActive(bool active)
{
	if (!active)
		m_actor->putToSleep();
	else
		m_actor->wakeUp();
}

bool BodyPhysX::isActive() const
{
	return !m_actor->isSleeping();
}

void BodyPhysX::setEnable(bool enable)
{
}

bool BodyPhysX::isEnable() const
{
	return true;
}

void BodyPhysX::reset()
{
	setLinearVelocity(Vector4::zero());
	setAngularVelocity(Vector4::zero());
}

void BodyPhysX::setMass(float mass, const Vector4& inertiaTensor)
{
}

float BodyPhysX::getInverseMass() const
{
	return 1.0f / m_actor->getMass();
}

Matrix33 BodyPhysX::getInertiaTensorInverseWorld() const
{
	T_BREAKPOINT;
	return Matrix33::identity();
}

void BodyPhysX::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 force_ = convert(this, force, localSpace);

	m_actor->addForceAtPos(
		toNxVec3(force_),
		toNxVec3(at_)
	);
}

void BodyPhysX::addTorque(const Vector4& torque, bool localSpace)
{
	Vector4 torque_ = convert(this, torque, localSpace);

	m_actor->addTorque(
		toNxVec3(torque_)
	);
}

void BodyPhysX::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	Vector4 momentum = fromNxVec3(m_actor->getLinearMomentum(), 0.0f);
	m_actor->setLinearMomentum(toNxVec3(momentum + linearImpulse_));
}

void BodyPhysX::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	T_BREAKPOINT;
}

void BodyPhysX::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	T_BREAKPOINT;
}

void BodyPhysX::setLinearVelocity(const Vector4& linearVelocity)
{
	m_actor->setLinearVelocity(toNxVec3(linearVelocity));
}

Vector4 BodyPhysX::getLinearVelocity() const
{
	return fromNxVec3(m_actor->getLinearVelocity(), 0.0f);
}

void BodyPhysX::setAngularVelocity(const Vector4& angularVelocity)
{
	m_actor->setAngularVelocity(toNxVec3(angularVelocity));
}

Vector4 BodyPhysX::getAngularVelocity() const
{
	return fromNxVec3(m_actor->getAngularVelocity(), 0.0f);
}

Vector4 BodyPhysX::getVelocityAt(const Vector4& at, bool localSpace) const
{
	Vector4 at_ = convert(this, at, localSpace);
	return fromNxVec3(m_actor->getPointVelocity(toNxVec3(at_)), 0.0f);
}

bool BodyPhysX::setState(const BodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
	return true;
}

BodyState BodyPhysX::getState() const
{
	BodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

	}
}

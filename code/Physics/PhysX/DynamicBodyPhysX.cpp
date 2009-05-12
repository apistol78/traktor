#include <NxPhysics.h>
#include "Physics/PhysX/DynamicBodyPhysX.h"
#include "Physics/PhysX/Conversion.h"
#include "Core/Math/Float.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

inline Vector4 convert(const DynamicBodyPhysX* body, const Vector4& v, bool localSpace)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan((v).w()));
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DynamicBodyPhysX", DynamicBodyPhysX, DynamicBody)

DynamicBodyPhysX::DynamicBodyPhysX(DestroyCallbackPhysX* callback, NxActor* actor)
:	BodyPhysX< DynamicBody >(callback, actor)
{
}

void DynamicBodyPhysX::setTransform(const Matrix44& transform)
{
	m_actor->setGlobalPose(toNxMat34(transform));
}

Matrix44 DynamicBodyPhysX::getTransform() const
{
	return fromNxMat34(m_actor->getGlobalPose());
}

void DynamicBodyPhysX::reset()
{
	setLinearVelocity(Vector4::zero());
	setAngularVelocity(Vector4::zero());
}

void DynamicBodyPhysX::setMass(float mass, const Vector4& inertiaTensor)
{
}

float DynamicBodyPhysX::getInverseMass() const
{
	return 1.0f / m_actor->getMass();
}

Matrix33 DynamicBodyPhysX::getInertiaTensorInverseWorld() const
{
	T_BREAKPOINT;
	return Matrix33::identity();
}

void DynamicBodyPhysX::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 force_ = convert(this, force, localSpace);

	m_actor->addForceAtPos(
		toNxVec3(force_),
		toNxVec3(at_)
	);
}

void DynamicBodyPhysX::addTorque(const Vector4& torque, bool localSpace)
{
	Vector4 torque_ = convert(this, torque, localSpace);

	m_actor->addTorque(
		toNxVec3(torque_)
	);
}

void DynamicBodyPhysX::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	Vector4 momentum = fromNxVec3(m_actor->getLinearMomentum(), 0.0f);
	m_actor->setLinearMomentum(toNxVec3(momentum + linearImpulse_));
}

void DynamicBodyPhysX::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	T_BREAKPOINT;
}

void DynamicBodyPhysX::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	T_BREAKPOINT;
}

void DynamicBodyPhysX::setLinearVelocity(const Vector4& linearVelocity)
{
	m_actor->setLinearVelocity(toNxVec3(linearVelocity));
}

Vector4 DynamicBodyPhysX::getLinearVelocity() const
{
	return fromNxVec3(m_actor->getLinearVelocity(), 0.0f);
}

void DynamicBodyPhysX::setAngularVelocity(const Vector4& angularVelocity)
{
	m_actor->setAngularVelocity(toNxVec3(angularVelocity));
}

Vector4 DynamicBodyPhysX::getAngularVelocity() const
{
	return fromNxVec3(m_actor->getAngularVelocity(), 0.0f);
}

Vector4 DynamicBodyPhysX::getVelocityAt(const Vector4& at, bool localSpace) const
{
	Vector4 at_ = convert(this, at, localSpace);
	return fromNxVec3(m_actor->getPointVelocity(toNxVec3(at_)), 0.0f);
}

bool DynamicBodyPhysX::setState(const DynamicBodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
	return true;
}

DynamicBodyState DynamicBodyPhysX::getState() const
{
	DynamicBodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void DynamicBodyPhysX::setEnable(bool enable)
{
	if (!enable)
		m_actor->putToSleep();
	else
		m_actor->wakeUp();
}

bool DynamicBodyPhysX::getEnable() const
{
	return !m_actor->isSleeping();
}

	}
}

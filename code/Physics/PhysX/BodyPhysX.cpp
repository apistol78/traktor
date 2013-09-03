#include <PxPhysicsAPI.h>
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

BodyPhysX::BodyPhysX(
	DestroyCallbackPhysX* callback,
	physx::PxScene* scene,
	physx::PxRigidActor* actor,
	const Vector4& centerOfGravity
)
:	m_callback(callback)
,	m_scene(scene)
,	m_actor(actor)
,	m_centerOfGravity(centerOfGravity)
,	m_enabled(false)
{
}

void BodyPhysX::destroy()
{
	if (m_callback)
	{
		m_callback->destroyBody(this, m_actor);
		m_callback = 0;
	}
	m_actor = 0;
	Body::destroy();
}

void BodyPhysX::setTransform(const Transform& transform)
{
	m_actor->setGlobalPose(toPxTransform(transform * Transform(m_centerOfGravity)));
}

Transform BodyPhysX::getTransform() const
{
	return fromPxTransform(m_actor->getGlobalPose()) * Transform(-m_centerOfGravity);
}

Transform BodyPhysX::getCenterTransform() const
{
	return fromPxTransform(m_actor->getGlobalPose());
}

bool BodyPhysX::isStatic() const
{
	return m_actor->is< physx::PxRigidStatic >() != 0;
}

bool BodyPhysX::isKinematic() const
{
	return m_actor->is< physx::PxRigidStatic >() != 0;
}

void BodyPhysX::setActive(bool active)
{
	physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
	if (!active)
		bodyDynamic->putToSleep();
	else
		bodyDynamic->wakeUp();
}

bool BodyPhysX::isActive() const
{
	physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
	return !bodyDynamic->isSleeping();
}

void BodyPhysX::setEnable(bool enable)
{
	if (enable != m_enabled)
	{
		if (enable)
			m_scene->addActor(*m_actor);
		else
			m_scene->removeActor(*m_actor);

		m_enabled = enable;
	}
}

bool BodyPhysX::isEnable() const
{
	return m_enabled;
}

void BodyPhysX::reset()
{
	setLinearVelocity(Vector4::zero());
	setAngularVelocity(Vector4::zero());
}

void BodyPhysX::setMass(float mass, const Vector4& inertiaTensor)
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	rigidBody->setMass(mass);
	rigidBody->setMassSpaceInertiaTensor(toPxVec3(inertiaTensor));
}

float BodyPhysX::getInverseMass() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	return 1.0f / rigidBody->getMass();
}

Matrix33 BodyPhysX::getInertiaTensorInverseWorld() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	physx::PxVec3 inertiaTensor = rigidBody->getMassSpaceInertiaTensor();
	return Matrix33(
		inertiaTensor.x, 0.0f, 0.0f,
		0.0f, inertiaTensor.y, 0.0f,
		0.0f, 0.0f, inertiaTensor.z
	).inverse();
}

void BodyPhysX::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	Vector4 at0 = (localSpace ? (at + m_centerOfGravity) : at).xyz1();
	Vector4 at_ = convert(this, at0.xyz1(), localSpace);
	Vector4 force_ = convert(this, force.xyz0(), localSpace);
	physx::PxRigidBodyExt::addForceAtPos(
		*m_actor->is< physx::PxRigidBody >(),
		toPxVec3(force_),
		toPxVec3(at_)
	);
}

void BodyPhysX::addTorque(const Vector4& torque, bool localSpace)
{
	physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
	Vector4 torque_ = convert(this, torque.xyz0(), localSpace);
	bodyDynamic->addTorque(
		toPxVec3(torque_)
	);
}

void BodyPhysX::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	Vector4 linearImpulse_ = convert(this, linearImpulse.xyz0(), localSpace);
	Vector4 momentum = fromPxVec3(rigidBody->getLinearVelocity(), 0.0f);
	rigidBody->setLinearVelocity(toPxVec3(momentum + linearImpulse_));
}

void BodyPhysX::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	Vector4 angularImpulse_ = convert(this, angularImpulse.xyz0(), localSpace);
	Vector4 momentum = fromPxVec3(rigidBody->getAngularVelocity(), 0.0f);
	rigidBody->setAngularVelocity(toPxVec3(momentum + angularImpulse_));
}

void BodyPhysX::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	if (localSpace)
	{
		addLinearImpulse(impulse, true);
		addAngularImpulse(cross(at, impulse), true);
	}
	else
	{
		Transform Tinv = getTransform().inverse();
		Vector4 localAt = Tinv * at.xyz1();
		Vector4 localImpulse = Tinv * impulse.xyz0();
		addLinearImpulse(localImpulse, true);
		addAngularImpulse(cross(localAt, localImpulse), true);
	}
}

void BodyPhysX::setLinearVelocity(const Vector4& linearVelocity)
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	rigidBody->setLinearVelocity(toPxVec3(linearVelocity));
}

Vector4 BodyPhysX::getLinearVelocity() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	return fromPxVec3(rigidBody->getLinearVelocity(), 0.0f);
}

void BodyPhysX::setAngularVelocity(const Vector4& angularVelocity)
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	rigidBody->setAngularVelocity(toPxVec3(angularVelocity));
}

Vector4 BodyPhysX::getAngularVelocity() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	return fromPxVec3(rigidBody->getAngularVelocity(), 0.0f);
}

Vector4 BodyPhysX::getVelocityAt(const Vector4& at, bool localSpace) const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	Vector4 at_ = convert(this, at, localSpace);
	physx::PxVec3 velocity = physx::PxRigidBodyExt::getVelocityAtPos(*rigidBody, toPxVec3(at_));
	return fromPxVec3(velocity, 0.0f);
}

bool BodyPhysX::solveStateConstraint(const BodyState& state)
{
	return false;
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

void BodyPhysX::integrate(float deltaTime)
{
}

	}
}

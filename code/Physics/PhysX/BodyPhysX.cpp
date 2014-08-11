#include <PxPhysicsAPI.h>
#include "Core/Math/Float.h"
#include "Core/Misc/TString.h"
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
	IWorldCallback* callback,
	physx::PxScene* scene,
	physx::PxRigidActor* actor,
	const Vector4& centerOfGravity,
	uint32_t collisionGroup,
	uint32_t collisionMask,
	int32_t material,
	const wchar_t* const tag
)
:	m_callback(callback)
,	m_scene(scene)
,	m_actor(actor)
,	m_centerOfGravity(centerOfGravity)
,	m_collisionGroup(collisionGroup)
,	m_collisionMask(collisionMask)
,	m_material(material)
,	m_enabled(false)
{
	if (tag)
		m_tag = wstombs(tag);
	else
		m_tag = "";

	actor->setName(m_tag.c_str());
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
	physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
	if (bodyDynamic)
		return bodyDynamic->getRigidDynamicFlags().isSet(physx::PxRigidDynamicFlag::eKINEMATIC);
	else
		return false;
}

void BodyPhysX::setActive(bool active)
{
	if (m_enabled)
	{
		physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
		if (bodyDynamic)
		{
			if (!active)
				bodyDynamic->putToSleep();
			else
				bodyDynamic->wakeUp();
		}
	}
}

bool BodyPhysX::isActive() const
{
	if (m_enabled)
	{
		physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
		return bodyDynamic ? !bodyDynamic->isSleeping() : true;
	}
	else
		return false;
}

void BodyPhysX::setEnable(bool enable)
{
	if (enable != m_enabled)
	{
		if (enable)
			m_callback->insertActor(m_actor);
		else
			m_callback->removeActor(m_actor);

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
	if (rigidBody)
	{
		rigidBody->setMass(mass);
		rigidBody->setMassSpaceInertiaTensor(toPxVec3(inertiaTensor));
	}
}

float BodyPhysX::getInverseMass() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	return rigidBody ? 1.0f / rigidBody->getMass() : 0.0f;
}

Matrix33 BodyPhysX::getInertiaTensorInverseWorld() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
	{
		physx::PxVec3 inertiaTensor = rigidBody->getMassSpaceInertiaTensor();
		return Matrix33(
			inertiaTensor.x, 0.0f, 0.0f,
			0.0f, inertiaTensor.y, 0.0f,
			0.0f, 0.0f, inertiaTensor.z
		).inverse();
	}
	else
		return Matrix33::identity();
}

void BodyPhysX::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	if (!m_enabled)
		return;

	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
	{
		Vector4 at0 = (localSpace ? (at + m_centerOfGravity) : at).xyz1();
		Vector4 at_ = convert(this, at0.xyz1(), localSpace);
		Vector4 force_ = convert(this, force.xyz0(), localSpace);
		physx::PxRigidBodyExt::addForceAtPos(
			*rigidBody,
			toPxVec3(force_),
			toPxVec3(at_)
		);
	}
}

void BodyPhysX::addTorque(const Vector4& torque, bool localSpace)
{
	if (!m_enabled)
		return;

	physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
	if (bodyDynamic)
	{
		Vector4 torque_ = convert(this, torque.xyz0(), localSpace);
		bodyDynamic->addTorque(toPxVec3(torque_));
	}
}

void BodyPhysX::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	if (!m_enabled)
		return;

	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
	{
		Vector4 linearImpulse_ = convert(this, linearImpulse.xyz0(), localSpace);
		rigidBody->addForce(toPxVec3(linearImpulse_), physx::PxForceMode::eIMPULSE);
	}
}

void BodyPhysX::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	if (!m_enabled)
		return;

	physx::PxRigidDynamic* bodyDynamic = m_actor->is< physx::PxRigidDynamic >();
	if (bodyDynamic)
	{
		Vector4 angularImpulse_ = convert(this, angularImpulse.xyz0(), localSpace);
		bodyDynamic->addTorque(
			toPxVec3(angularImpulse_),
			physx::PxForceMode::eIMPULSE
		);
	}
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
	if (rigidBody)
		rigidBody->setLinearVelocity(toPxVec3(linearVelocity));
}

Vector4 BodyPhysX::getLinearVelocity() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
		return fromPxVec3(rigidBody->getLinearVelocity(), 0.0f);
	else
		return Vector4::zero();
}

void BodyPhysX::setAngularVelocity(const Vector4& angularVelocity)
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
		rigidBody->setAngularVelocity(toPxVec3(angularVelocity));
}

Vector4 BodyPhysX::getAngularVelocity() const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
		return fromPxVec3(rigidBody->getAngularVelocity(), 0.0f);
	else
		return Vector4::zero();
}

Vector4 BodyPhysX::getVelocityAt(const Vector4& at, bool localSpace) const
{
	physx::PxRigidBody* rigidBody = m_actor->is< physx::PxRigidBody >();
	if (rigidBody)
	{
		Vector4 at_ = convert(this, at, localSpace);
		physx::PxVec3 velocity = physx::PxRigidBodyExt::getVelocityAtPos(*rigidBody, toPxVec3(at_));
		return fromPxVec3(velocity, 0.0f);
	}
	else
		return Vector4::zero();
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

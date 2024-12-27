/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Physics/BodyState.h"
#include "Physics/Joint.h"
#include "Physics/Mesh.h"
#include "Physics/Jolt/Conversion.h"
#include "Physics/Jolt/BodyJolt.h"
#include "Physics/Jolt/Types.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyJolt", BodyJolt, Body)

BodyJolt::BodyJolt(
	const wchar_t* const tag,
	IWorldCallback* callback,
	JPH::PhysicsSystem* physicsSystem,
	JPH::Body* body,
	uint32_t collisionGroup,
	uint32_t collisionMask
)
:	Body(tag)
,	m_callback(callback)
,	m_physicsSystem(physicsSystem)
,	m_body(body)
,	m_collisionGroup(collisionGroup)
,	m_collisionMask(collisionMask)
{
	m_body->SetUserData((JPH::uint64)this);
}

void BodyJolt::destroy()
{
	if (m_callback)
	{
		m_callback->destroyBody(this);
		m_callback = nullptr;

		JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
		bodyInterface.RemoveBody(m_body->GetID());
		bodyInterface.DestroyBody(m_body->GetID());
		Body::destroy();

		m_body = nullptr;
	}
}

void BodyJolt::setTransform(const Transform& transform)
{
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetPositionAndRotation(
		m_body->GetID(),
		convertToJolt(transform.translation()),
		convertToJolt(transform.rotation()),
		JPH::EActivation::Activate
	);
}

Transform BodyJolt::getTransform() const
{
	const JPH::RMat44 transform = m_body->GetWorldTransform();
	return convertFromJolt(transform);
}

Transform BodyJolt::getCenterTransform() const
{
	const JPH::RMat44 transform = m_body->GetCenterOfMassTransform();
	return convertFromJolt(transform);
}

void BodyJolt::setKinematic(bool kinematic)
{
}

bool BodyJolt::isStatic() const
{
	return m_body->IsStatic();
}

bool BodyJolt::isKinematic() const
{
	return m_body->IsKinematic();
}

void BodyJolt::setActive(bool active)
{
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	if (active)
		bodyInterface.ActivateBody(m_body->GetID());
	else
		bodyInterface.DeactivateBody(m_body->GetID());
}

bool BodyJolt::isActive() const
{
	return m_body->IsActive();
}

void BodyJolt::setEnable(bool enable)
{
}

bool BodyJolt::isEnable() const
{
	return false;
}

void BodyJolt::reset()
{
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetLinearVelocity(m_body->GetID(), JPH::Vec3::sReplicate(0.0f));
	bodyInterface.SetAngularVelocity(m_body->GetID(), JPH::Vec3::sReplicate(0.0f));
}

void BodyJolt::setMass(float mass, const Vector4& inertiaTensor)
{
}

float BodyJolt::getInverseMass() const
{
	return 1.0f;
}

Matrix33 BodyJolt::getInertiaTensorInverseWorld() const
{
	return Matrix33::identity();
}

void BodyJolt::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	Vector4 wat = at;
	Vector4 wforce = force;

	if (localSpace)
	{
		const Transform transform = getTransform();
		wat = transform * at;
		wforce = transform * force;
	}

	m_body->AddForce(convertToJolt(wforce), convertToJolt(wat));
}

void BodyJolt::addTorque(const Vector4& torque, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddTorque(convertToJolt(torque));
}

void BodyJolt::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddImpulse(convertToJolt(linearImpulse));
}

void BodyJolt::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddAngularImpulse(convertToJolt(angularImpulse));
}

void BodyJolt::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddImpulse(convertToJolt(impulse), convertToJolt(at));
}

void BodyJolt::setLinearVelocity(const Vector4& linearVelocity)
{
}

Vector4 BodyJolt::getLinearVelocity() const
{
	return convertFromJolt(m_body->GetLinearVelocity(), 0.0f);
}

void BodyJolt::setAngularVelocity(const Vector4& angularVelocity)
{
}

Vector4 BodyJolt::getAngularVelocity() const
{
	return convertFromJolt(m_body->GetAngularVelocity(), 0.0f);
}

Vector4 BodyJolt::getVelocityAt(const Vector4& at, bool localSpace) const
{
	Vector4 wat = at;
	if (localSpace)
		wat = getTransform() * at;
	return convertFromJolt(m_body->GetPointVelocity(convertToJolt(at)), 0.0f);
}

void BodyJolt::setState(const BodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
}

BodyState BodyJolt::getState() const
{
	BodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void BodyJolt::integrate(float deltaTime)
{
}

}

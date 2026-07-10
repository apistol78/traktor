/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Constraints/Constraint.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Physics/Jolt/BodyJolt.h"

#include "Core/Math/Float.h"
#include "Physics/BodyState.h"
#include "Physics/Mesh.h"
#include "Physics/Jolt/Conversion.h"

#include <algorithm>

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyJolt", BodyJolt, Body)

BodyJolt::BodyJolt(
	const wchar_t* const tag,
	IWorldCallback* callback,
	JPH::PhysicsSystem* physicsSystem,
	JPH::Body* body,
	float inverseMass,
	const Vector4& centerOfGravity,
	uint32_t collisionGroup,
	uint32_t collisionMask,
	int32_t material,
	const resource::Proxy< Mesh >& mesh)
:	Body(tag)
,	m_callback(callback)
,	m_physicsSystem(physicsSystem)
,	m_body(body)
,	m_inverseMass(inverseMass)
,	m_centerOfGravity(centerOfGravity)
,	m_collisionGroup(collisionGroup)
,	m_collisionMask(collisionMask)
,	m_material(material)
,	m_mesh(mesh)
{
	m_body->SetUserData((JPH::uint64)this);
}

void BodyJolt::destroy()
{
	if (!m_callback)
		return;

	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	const JPH::BodyID bodyID = m_body->GetID();

	for (auto constraint : m_constraints)
		m_callback->removeConstraint(constraint);
	m_constraints.clear();

	if (m_enabled)
	{
		bodyInterface.RemoveBody(bodyID);
		m_enabled = false;
	}
	bodyInterface.DestroyBody(bodyID);

	m_callback->destroyBody(this);
	m_callback = nullptr;
	m_body = nullptr;

	Body::destroy();
}

void BodyJolt::setTransform(const Transform& transform)
{
	if (!m_body)
		return;

	const Transform ct = transform * Transform(m_centerOfGravity);

	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetPositionAndRotation(
		m_body->GetID(),
		convertToJolt(ct.translation()),
		convertToJolt(ct.rotation()),
		isKinematic() ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

Transform BodyJolt::getTransform() const
{
	if (!m_body)
		return Transform::identity();
	return convertFromJoltAsTransform(m_body->GetWorldTransform()) * Transform(-m_centerOfGravity);
}

Transform BodyJolt::getCenterTransform() const
{
	if (!m_body)
		return Transform::identity();
	return convertFromJoltAsTransform(m_body->GetWorldTransform());
}

void BodyJolt::setKinematic(bool kinematic)
{
	if (!m_body || m_body->IsStatic())
		return;
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetMotionType(
		m_body->GetID(),
		kinematic ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic,
		JPH::EActivation::Activate);
}

bool BodyJolt::isStatic() const
{
	return m_body ? m_body->IsStatic() : false;
}

bool BodyJolt::isKinematic() const
{
	return m_body ? m_body->IsKinematic() : false;
}

void BodyJolt::setActive(bool active)
{
	if (!m_body)
		return;
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	if (active)
		bodyInterface.ActivateBody(m_body->GetID());
	else
		bodyInterface.DeactivateBody(m_body->GetID());
}

bool BodyJolt::isActive() const
{
	return m_body ? m_body->IsActive() : false;
}

void BodyJolt::setEnable(bool enable)
{
	if (!m_body || enable == m_enabled)
		return;

	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	if (enable)
	{
		bodyInterface.AddBody(m_body->GetID(), m_body->IsStatic() ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
		for (auto constraint : m_constraints)
			m_callback->insertConstraint(constraint);
	}
	else
	{
		for (auto constraint : m_constraints)
			m_callback->removeConstraint(constraint);
		bodyInterface.RemoveBody(m_body->GetID());
	}
	m_enabled = enable;
}

bool BodyJolt::isEnable() const
{
	return m_enabled;
}

void BodyJolt::reset()
{
	if (!m_body || m_body->IsStatic())
		return;
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetLinearAndAngularVelocity(m_body->GetID(), JPH::Vec3::sZero(), JPH::Vec3::sZero());
}

void BodyJolt::setMass(float mass, const Vector4& inertiaTensor)
{
	if (!m_body || m_body->IsStatic())
		return;

	T_ASSERT(!isNanOrInfinite(mass));

	JPH::MotionProperties* mp = m_body->GetMotionProperties();
	if (!mp)
		return;

	mp->SetInverseMass(mass > 0.0f ? 1.0f / mass : 0.0f);
	mp->SetInverseInertia(
		JPH::Vec3(
			inertiaTensor.x() > 0.0f ? 1.0f / inertiaTensor.x() : 0.0f,
			inertiaTensor.y() > 0.0f ? 1.0f / inertiaTensor.y() : 0.0f,
			inertiaTensor.z() > 0.0f ? 1.0f / inertiaTensor.z() : 0.0f),
		JPH::Quat::sIdentity());

	m_inverseMass = mass > 0.0f ? 1.0f / mass : 0.0f;
}

float BodyJolt::getInverseMass() const
{
	return m_inverseMass;
}

Matrix33 BodyJolt::getInertiaTensorInverseWorld() const
{
	if (!m_body)
		return Matrix33::identity();
	return convertFromJoltAsMatrix33(m_body->GetInverseInertia());
}

void BodyJolt::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	if (!m_body)
		return;

	Vector4 wat = at;
	Vector4 wforce = force;
	if (localSpace)
	{
		const Transform transform = getTransform();
		wat = transform * (at + m_centerOfGravity);
		wforce = transform * force;
	}

	// Via the body interface so an inactive body is woken; the raw Body::AddForce does
	// not activate and its contribution would be lost on a sleeping body.
	m_physicsSystem->GetBodyInterface().AddForce(m_body->GetID(), convertToJolt(wforce), convertToJolt(wat));
}

void BodyJolt::addTorque(const Vector4& torque, bool localSpace)
{
	if (!m_body)
		return;

	Vector4 wtorque = torque;
	if (localSpace)
		wtorque = getTransform() * torque;

	m_physicsSystem->GetBodyInterface().AddTorque(m_body->GetID(), convertToJolt(wtorque));
}

void BodyJolt::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	if (!m_body)
		return;

	Vector4 wimpulse = linearImpulse;
	if (localSpace)
		wimpulse = getTransform() * linearImpulse;

	// Via the body interface so a sleeping body is woken; the raw Body::AddImpulse does
	// not activate and its impulse would be lost on an inactive body.
	m_physicsSystem->GetBodyInterface().AddImpulse(m_body->GetID(), convertToJolt(wimpulse));
}

void BodyJolt::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	if (!m_body)
		return;

	Vector4 wimpulse = angularImpulse;
	if (localSpace)
		wimpulse = getTransform() * angularImpulse;

	m_physicsSystem->GetBodyInterface().AddAngularImpulse(m_body->GetID(), convertToJolt(wimpulse));
}

void BodyJolt::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	if (!m_body)
		return;

	Vector4 wat = at;
	Vector4 wimpulse = impulse;
	if (localSpace)
	{
		const Transform transform = getTransform();
		wat = transform * (at + m_centerOfGravity);
		wimpulse = transform * impulse;
	}

	m_physicsSystem->GetBodyInterface().AddImpulse(m_body->GetID(), convertToJolt(wimpulse), convertToJolt(wat));
}

void BodyJolt::setLinearVelocity(const Vector4& linearVelocity)
{
	if (!m_body || m_body->IsStatic())
		return;
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetLinearVelocity(m_body->GetID(), convertToJolt(linearVelocity));
}

Vector4 BodyJolt::getLinearVelocity() const
{
	return m_body ? convertFromJolt(m_body->GetLinearVelocity(), 0.0f) : Vector4::zero();
}

void BodyJolt::setAngularVelocity(const Vector4& angularVelocity)
{
	if (!m_body || m_body->IsStatic())
		return;
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetAngularVelocity(m_body->GetID(), convertToJolt(angularVelocity));
}

Vector4 BodyJolt::getAngularVelocity() const
{
	return m_body ? convertFromJolt(m_body->GetAngularVelocity(), 0.0f) : Vector4::zero();
}

Vector4 BodyJolt::getVelocityAt(const Vector4& at, bool localSpace) const
{
	if (!m_body)
		return Vector4::zero();

	Vector4 wat = at;
	if (localSpace)
		wat = getTransform() * at;

	return convertFromJolt(m_body->GetPointVelocity(convertToJolt(wat)), 0.0f);
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
	if (!m_body || m_body->IsStatic())
		return;

	JPH::MotionProperties* mp = m_body->GetMotionProperties();
	if (!mp)
		return;

	const JPH::Vec3 gravity = m_physicsSystem->GetGravity();

	JPH::Vec3 linearVelocity = mp->GetLinearVelocity() + gravity * deltaTime;
	JPH::Vec3 angularVelocity = mp->GetAngularVelocity();

	// Apply damping.
	linearVelocity *= JPH::max(0.0f, 1.0f - mp->GetLinearDamping() * deltaTime);
	angularVelocity *= JPH::max(0.0f, 1.0f - mp->GetAngularDamping() * deltaTime);

	mp->SetLinearVelocity(linearVelocity);
	mp->SetAngularVelocity(angularVelocity);

	JPH::RVec3 position = m_body->GetCenterOfMassPosition() + JPH::RVec3(linearVelocity) * deltaTime;
	JPH::Quat rotation = m_body->GetRotation();
	if (angularVelocity.LengthSq() > 0.0f)
	{
		const float omega = angularVelocity.Length();
		const JPH::Quat dq = JPH::Quat::sRotation(angularVelocity / omega, omega * deltaTime);
		rotation = (dq * rotation).Normalized();
	}

	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetPositionAndRotation(m_body->GetID(), position, rotation, JPH::EActivation::Activate);
}

void BodyJolt::addConstraint(JPH::Constraint* constraint)
{
	m_constraints.push_back(constraint);
	if (m_enabled && m_callback)
		m_callback->insertConstraint(constraint);
}

void BodyJolt::removeConstraint(JPH::Constraint* constraint)
{
	auto it = std::find(m_constraints.begin(), m_constraints.end(), constraint);
	if (it == m_constraints.end())
		return;
	if (m_enabled && m_callback)
		m_callback->removeConstraint(constraint);
	m_constraints.erase(it);
}

void BodyJolt::getFrictionAndRestitution(const JPH::SubShapeID& subShapeID, float& outFriction, float& outRestitution) const
{
	outFriction = m_body ? m_body->GetFriction() : 0.0f;
	outRestitution = m_body ? m_body->GetRestitution() : 0.0f;

	if (!m_body || !m_mesh)
		return;

	// SubShapeID is an opaque BVH path — resolve it through any wrapping
	// (e.g. compound) shapes to the leaf MeshShape, then ask Jolt for the
	// material index it stored for the hit triangle.
	JPH::SubShapeID remainder;
	const JPH::Shape* leaf = m_body->GetShape()->GetLeafShape(subShapeID, remainder);
	if (!leaf || leaf->GetSubType() != JPH::EShapeSubType::Mesh)
		return;

	const auto* meshShape = static_cast< const JPH::MeshShape* >(leaf);
	const uint32_t midx = meshShape->GetMaterialIndex(remainder);

	const auto& materials = m_mesh->getMaterials();
	if (midx < materials.size())
	{
		outFriction = materials[midx].friction;
		outRestitution = materials[midx].restitution;
	}
}

}

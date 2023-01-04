/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/World/RigidBodyComponent.h"
#include "World/Entity.h"
#include "World/EntityEventManager.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidBodyComponent", RigidBodyComponent, world::IEntityComponent)

RigidBodyComponent::RigidBodyComponent(
	Body* body,
	world::EntityEventManager* eventManager,
	world::IEntityEvent* eventCollide
)
:	m_owner(nullptr)
,	m_body(body)
,	m_eventManager(eventManager)
,	m_eventCollide(eventCollide)
{
	if (m_body && m_eventCollide)
		m_body->addCollisionListener(physics::createCollisionListener(this, &RigidBodyComponent::collisionListener));
}

void RigidBodyComponent::destroy()
{
	m_eventCollide = nullptr;
	m_eventManager = nullptr;
	safeDestroy(m_body);
	m_owner = nullptr;
}

void RigidBodyComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
	{
		const Transform transform = m_owner->getTransform();
		m_body->setTransform(transform);
		m_body->setEnable(true);
	}
}

void RigidBodyComponent::setTransform(const Transform& transform)
{
	m_body->setTransform(transform);
}

Aabb3 RigidBodyComponent::getBoundingBox() const
{
	return Aabb3();
}

void RigidBodyComponent::update(const world::UpdateParams& update)
{
	if (!m_body->isActive() ||m_body->isStatic() || m_body->isKinematic())
		return;

	// Body is dynamic and active; need to update owner's transform.
	m_owner->setTransform(m_body->getTransform());
}

void RigidBodyComponent::collisionListener(const physics::CollisionInfo& collisionInfo)
{
	Vector4 position = Vector4::zero();
	Vector4 normal = Vector4::zero();

	for (const auto& contact : collisionInfo.contacts)
	{
		position += contact.position;
		normal += contact.normal;
	}

	position = position / Scalar(float(collisionInfo.contacts.size()));
	normal = normal.normalized();

	const Transform Tworld(
		position,
		Quaternion(Vector4(0.0f, 1.0f, 0.0f, 0.0f), normal)
	);

	const Transform T = m_body->getTransform();
	m_eventManager->raise(m_eventCollide, m_owner, T.inverse() * Tworld);
}

}

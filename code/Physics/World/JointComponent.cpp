/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/StaticVector.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Joint.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/JointComponent.h"
#include "Physics/World/RigidBodyComponent.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.JointComponent", JointComponent, world::IEntityComponent)

JointComponent::JointComponent(PhysicsManager* physicsManager, const JointDesc* jointDesc)
:	m_physicsManager(physicsManager)
,	m_jointDesc(jointDesc)
{
}

void JointComponent::destroy()
{
	safeDestroy(m_joint);
	m_physicsManager = nullptr;
}

void JointComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void JointComponent::setTransform(const Transform& transform)
{
}

Aabb3 JointComponent::getBoundingBox() const
{
	return Aabb3();
}

void JointComponent::update(const world::UpdateParams& update)
{
	// Lazy create joint.
	if (!m_joint)
	{
		StaticVector< Body*, 2 > bodies;

		auto groupComponent = m_owner->getComponent< world::GroupComponent >();
		if (groupComponent)
		{
			// Gather rigid bodies.
			for (auto entity : groupComponent->getEntities())
			{
				auto rigidBodyComponent = entity->getComponent< RigidBodyComponent >();
				if (rigidBodyComponent)
				{
					bodies.push_back(rigidBodyComponent->getBody());
					if (bodies.full())
						break;
				}
			}
		}
		else
		{
			// No group component, use body with world.
			auto rigidBodyComponent = m_owner->getComponent< RigidBodyComponent >();
			if (rigidBodyComponent)
				bodies.push_back(rigidBodyComponent->getBody());
		}

		while (!bodies.full())
			bodies.push_back(nullptr);

		if (bodies[0])
			m_joint = m_physicsManager->createJoint(m_jointDesc, m_owner->getTransform(), bodies[0], bodies[1]);
	}
}

}

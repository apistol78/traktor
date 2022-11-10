/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Physics/PhysicsManager.h"
#include "Physics/CollisionListener.h"
#include "Physics/Body.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsManager", PhysicsManager, Object)

void PhysicsManager::addCollisionListener(CollisionListener* collisionListener)
{
	m_collisionListeners.push_back(collisionListener);
}

void PhysicsManager::removeCollisionListener(CollisionListener* collisionListener)
{
	auto it = std::find(m_collisionListeners.begin(), m_collisionListeners.end(), collisionListener);
	if (it != m_collisionListeners.end())
		m_collisionListeners.erase(it);
}

void PhysicsManager::notifyCollisionListeners(const CollisionInfo& collisionInfo)
{
	// Notify global listeners first.
	for (auto listener : m_collisionListeners)
		listener->notify(collisionInfo);

	// Also notify listeners attached to colliding bodies.
	if (collisionInfo.body1)
		collisionInfo.body1->notifyCollisionListeners(collisionInfo);
	if (collisionInfo.body2)
		collisionInfo.body2->notifyCollisionListeners(collisionInfo);
}

bool PhysicsManager::haveCollisionListeners() const
{
	return !m_collisionListeners.empty();
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
	RefArray< CollisionListener >::iterator i = std::find(m_collisionListeners.begin(), m_collisionListeners.end(), collisionListener);
	if (i != m_collisionListeners.end())
		m_collisionListeners.erase(i);
}

void PhysicsManager::notifyCollisionListeners(const CollisionInfo& collisionInfo)
{
	// Notify global listeners first.
	for (RefArray< CollisionListener >::iterator i = m_collisionListeners.begin(); i != m_collisionListeners.end(); ++i)
		(*i)->notify(collisionInfo);

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

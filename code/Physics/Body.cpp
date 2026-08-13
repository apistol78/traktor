/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Body.h"

#include "Physics/CollisionListener.h"

#include <algorithm>

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Body", Body, Object)

void Body::destroy()
{
	removeAllCollisionListeners();
	m_owner = nullptr;
}

void Body::addCollisionListener(CollisionListener* collisionListener)
{
	m_collisionListeners.push_back(collisionListener);
}

void Body::removeCollisionListener(CollisionListener* collisionListener)
{
	m_collisionListeners.remove(collisionListener);
}

void Body::removeAllCollisionListeners()
{
	m_collisionListeners.clear();
}

void Body::notifyCollisionListeners(const CollisionInfo& collisionInfo)
{
	for (auto listener : m_collisionListeners)
		listener->notify(collisionInfo);
}

Body::Body(const wchar_t* const tag)
	: m_tag(tag ? tag : L"")
	, m_owner(nullptr)
	, m_clusterId(~0U)
{
}

}

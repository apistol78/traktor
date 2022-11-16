/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/EntityEventManager.h"
#include "World/Entity/EventSetComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EventSetComponent", EventSetComponent, IEntityComponent)

EventSetComponent::EventSetComponent(EntityEventManager* eventManager)
:	m_eventManager(eventManager)
{
}

void EventSetComponent::destroy()
{
}

void EventSetComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void EventSetComponent::setTransform(const Transform& transform)
{
}

Aabb3 EventSetComponent::getBoundingBox() const
{
	return Aabb3();
}

void EventSetComponent::update(const UpdateParams& update)
{
}

void EventSetComponent::raise(const std::wstring& name)
{
	const IEntityEvent* event = getEvent(name);
	if (!event)
		return;

	m_eventManager->raise(event, m_owner, Transform::identity());
}

const IEntityEvent* EventSetComponent::getEvent(const std::wstring& name) const
{
	auto it = m_events.find(name);
	return it != m_events.end() ? it->second : nullptr;
}

}

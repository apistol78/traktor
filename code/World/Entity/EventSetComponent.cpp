/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/EntityEventManager.h"
#include "World/World.h"
#include "World/Entity/EventSetComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EventSetComponent", EventSetComponent, IEntityComponent)

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

IEntityEventInstance* EventSetComponent::raise(const std::wstring& name, const Transform& offset)
{
	EntityEventManager* eventManager = m_owner->getWorld()->getComponent< EntityEventManager >();
	if (!eventManager)
		return nullptr;

	const IEntityEvent* event = getEvent(name);
	if (!event)
		return nullptr;

	return eventManager->raise(event, m_owner, offset);
}

const IEntityEvent* EventSetComponent::getEvent(const std::wstring& name) const
{
	auto it = m_events.find(name);
	return it != m_events.end() ? it->second : nullptr;
}

}

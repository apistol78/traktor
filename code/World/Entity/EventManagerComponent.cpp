/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Acquire.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventInstance.h"
#include "World/Entity/EventManagerComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EventManagerComponent", EventManagerComponent, IWorldComponent)

EventManagerComponent::EventManagerComponent(uint32_t maxEventInstances)
:	m_maxEventInstances(maxEventInstances)
{
	m_eventInstances.reserve(maxEventInstances);
}

void EventManagerComponent::destroy()
{
	cancelAll(Cancel::Immediate);
}

void EventManagerComponent::update(World* world, const UpdateParams& update)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (auto it = m_eventInstances.begin(); it != m_eventInstances.end(); )
	{
		if ((*it)->update(world, update))
			++it;
		else
			it = m_eventInstances.erase(it);
	}
}

IEntityEventInstance* EventManagerComponent::raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!event || m_eventInstances.size() >= m_maxEventInstances)
		return nullptr;

	Ref< IEntityEventInstance > eventInstance = event->createInstance(this, sender, Toffset);
	if (eventInstance)
		m_eventInstances.push_back(eventInstance);

	return eventInstance;
}

void EventManagerComponent::cancelAll(Cancel when)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Issue cancel on all instances.
	for (auto eventInstance : m_eventInstances)
		eventInstance->cancel(when);

	// Remove all instances directly instead of waiting for next update.
	if (when == Cancel::Immediate)
		m_eventInstances.resize(0);
}

}

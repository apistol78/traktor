/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/IntervalTransformComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IntervalTransformComponent", IntervalTransformComponent, IWorldComponent)

void IntervalTransformComponent::destroy()
{
	T_FATAL_ASSERT_M(m_entries.empty(), L"Interval transform entries not empty.");
}

void IntervalTransformComponent::update(World* world, const UpdateParams& update)
{
	for (auto& entry : m_entries)
		entry->lastUpdate = entry->currentUpdate;
}

IntervalTransformComponent::Entry* IntervalTransformComponent::alloc()
{
	Entry* entry = new Entry();
	entry->currentUpdate = Transform::identity();
	entry->lastUpdate = Transform::identity();
	entry->currentRender = Transform::identity();
	entry->lastRender = Transform::identity();
	entry->index = (uint32_t)m_entries.size();
	m_entries.push_back(entry);
	return entry;
}

void IntervalTransformComponent::free(Entry*& entry)
{
	if (entry == nullptr)
		return;

	T_FATAL_ASSERT(entry->index < m_entries.size() && m_entries[entry->index] == entry);

	m_entries[entry->index] = m_entries.back();
	m_entries[entry->index]->index = entry->index;
	m_entries.pop_back();

	delete entry;
	entry = nullptr;
}

void IntervalTransformComponent::interpolate(float interval)
{
	const Scalar iv(interval);
	for (auto entry : m_entries)
	{
		entry->lastRender = entry->currentRender;
		entry->currentRender = lerp(
			entry->lastUpdate,
			entry->currentUpdate,
			iv);
	}
}

}

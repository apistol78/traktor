/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/EntityIdQuery.h"

#include "Render/Buffer.h"
#include "World/Entity.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityIdQuery", EntityIdQuery, Object)

void EntityIdQuery::request(const Vector2& position)
{
	m_state = State::Requested;
	m_position = position;
}

bool EntityIdQuery::poll(Ref< Entity >& outEntity)
{
	if (m_state != State::Rendered)
		return false;

	const float* slots = (const float*)m_readBackBuffer->lock();
	if (!slots)
		return false;
	const float value = slots[0];
	m_readBackBuffer->unlock();

	// Slot is reset to a negative value when rendered; still in flight until overwritten by the GPU.
	if (value < 0.0f)
		return false;

	const uint32_t id = (uint32_t)(value + 0.5f);
	if (id >= 1 && id < m_instanceBase)
		outEntity = m_entities[id - 1];
	else if (id >= m_instanceBase && id - m_instanceBase < m_instanceEntities.size())
		outEntity = m_instanceEntities[id - m_instanceBase];
	else
		outEntity = nullptr;

	m_entities.clear();
	m_instanceEntities.clear();
	m_state = State::Idle;
	return true;
}

}

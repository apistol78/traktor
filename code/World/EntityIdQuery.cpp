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

bool EntityIdQuery::poll(Ref< Entity >& outEntity, Vector4& outPosition)
{
	if (m_state != State::Rendered)
		return false;

	const float* slots = (const float*)m_readBackBuffer->lock();
	if (!slots)
		return false;
	const float value = slots[0];
	const float depth = slots[1];
	m_readBackBuffer->unlock();

	// Slot is reset to a negative value when rendered; still in flight until overwritten by the GPU.
	if (value < 0.0f)
		return false;

	// Unproject linear view depth; depth is zero if not available and far plane if nothing is rendered.
	if (depth > 0.0f && depth < m_viewFarZ)
	{
		const float ndcX = m_position.x * 2.0f - 1.0f;
		const float ndcY = 1.0f - m_position.y * 2.0f;
		const float w = m_projection.get(3, 2) * depth + m_projection.get(3, 3);
		const float x = (ndcX * w - m_projection.get(0, 2) * depth - m_projection.get(0, 3)) / m_projection.get(0, 0);
		const float y = (ndcY * w - m_projection.get(1, 2) * depth - m_projection.get(1, 3)) / m_projection.get(1, 1);
		outPosition = m_viewInverse * Vector4(x, y, depth, 1.0f);
	}
	else
		outPosition = Vector4::zero();

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

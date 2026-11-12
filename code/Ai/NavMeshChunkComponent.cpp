/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshChunkComponent.h"

#include "Ai/NavMeshChunk.h"
#include "Ai/NavMeshComponent.h"
#include "World/World.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshChunkComponent", NavMeshChunkComponent, world::IEntityComponent)

NavMeshChunkComponent::NavMeshChunkComponent(const resource::Proxy< NavMeshChunk >& chunk)
	: m_chunk(chunk)
{
}

void NavMeshChunkComponent::destroy()
{
	m_chunk.clear();
}

void NavMeshChunkComponent::setOwner(world::Entity* owner)
{
}

void NavMeshChunkComponent::setWorld(world::World* world)
{
	if (m_navMeshComponent)
	{
		m_navMeshComponent->removeChunk(m_chunk);
		m_navMeshComponent = nullptr;
	}

	if (world)
	{
		m_navMeshComponent = world->getComponent< NavMeshComponent >();
		if (m_navMeshComponent)
			m_navMeshComponent->addChunk(m_chunk);
	}
}

void NavMeshChunkComponent::setTransform(const Transform& transform)
{
}

Aabb3 NavMeshChunkComponent::getBoundingBox() const
{
	return Aabb3();
}

void NavMeshChunkComponent::update(const world::UpdateParams& update)
{
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshComponent.h"

#include "World/Entity.h"
#include "World/World.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponent", MeshComponent, world::IEntityComponent)

MeshComponent::~MeshComponent()
{
	freeIntervalTransform();
}

void MeshComponent::destroy()
{
	freeIntervalTransform();
	m_owner = nullptr;
}

void MeshComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void MeshComponent::setWorld(world::World* world)
{
	freeIntervalTransform();
	if ((m_world = world) != nullptr)
	{
		Ref< world::IntervalTransformComponent > intervalTransformComponent = m_world->getComponent< world::IntervalTransformComponent >();
		m_transform = intervalTransformComponent->alloc();
	}
}

void MeshComponent::setTransform(const Transform& transform)
{
	if (m_transform)
		m_transform->currentUpdate = transform;
}

void MeshComponent::update(const world::UpdateParams& update)
{
}

void MeshComponent::freeIntervalTransform()
{
	if (!m_world)
		return;

	if (m_transform)
	{
		Ref< world::IntervalTransformComponent > intervalTransformComponent = m_world->getComponent< world::IntervalTransformComponent >();
		if (intervalTransformComponent)
			intervalTransformComponent->free(m_transform);
		m_transform = nullptr;
	}
}

}

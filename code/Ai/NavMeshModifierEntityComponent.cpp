/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshModifierEntityComponent.h"

#include "Ai/NavMeshComponent.h"
#include "World/Entity.h"
#include "World/World.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshModifierEntityComponent", NavMeshModifierEntityComponent, world::IEntityComponent)

void NavMeshModifierEntityComponent::destroy()
{
	// Restore area since navigation mesh outlives modifier.
	if (m_navMeshComponent && !m_enable)
		m_navMeshComponent->setModifierEnable(m_owner->getId(), true);

	m_navMeshComponent = nullptr;
	m_owner = nullptr;
}

void NavMeshModifierEntityComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void NavMeshModifierEntityComponent::setTransform(const Transform& transform)
{
}

Aabb3 NavMeshModifierEntityComponent::getBoundingBox() const
{
	return Aabb3();
}

void NavMeshModifierEntityComponent::update(const world::UpdateParams& update)
{
}

void NavMeshModifierEntityComponent::setEnable(bool enable)
{
	if (!m_navMeshComponent)
	{
		const world::World* world = m_owner ? m_owner->getWorld() : nullptr;
		m_navMeshComponent = world ? world->getComponent< NavMeshComponent >() : nullptr;
		if (!m_navMeshComponent)
			return;
	}

	if (m_navMeshComponent->setModifierEnable(m_owner->getId(), enable))
		m_enable = enable;
}

}

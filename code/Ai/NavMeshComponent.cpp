/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshComponent.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponent", NavMeshComponent, world::IEntityComponent)

NavMeshComponent::NavMeshComponent(const resource::Proxy< NavMesh >& navMesh)
:	m_navMesh(navMesh)
{
}

void NavMeshComponent::destroy()
{
}

void NavMeshComponent::setOwner(world::Entity* owner)
{
}

void NavMeshComponent::setTransform(const Transform& transform)
{
}

Aabb3 NavMeshComponent::getBoundingBox() const
{
	return Aabb3();
}

void NavMeshComponent::update(const world::UpdateParams& update)
{
}

	}
}

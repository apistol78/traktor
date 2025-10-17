/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshComponent.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponent", NavMeshComponent, world::IWorldComponent)

NavMeshComponent::NavMeshComponent(const resource::Proxy< NavMesh >& navMesh)
:	m_navMesh(navMesh)
{
}

void NavMeshComponent::destroy()
{
}

void NavMeshComponent::update(world::World* world, const world::UpdateParams& update)
{
}

}

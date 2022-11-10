/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/TerrainLayerComponent.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainLayerComponent", TerrainLayerComponent, world::IEntityComponent)

void TerrainLayerComponent::setOwner(world::Entity* owner)
{
	m_dirty = true;
}

void TerrainLayerComponent::update(const world::UpdateParams& update)
{
	if (m_dirty)
	{
		m_dirty = false;
		updatePatches();
	}
}

}

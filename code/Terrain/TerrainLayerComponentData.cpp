/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/TerrainLayerComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainLayerComponentData", TerrainLayerComponentData, world::IEntityComponentData)

int32_t TerrainLayerComponentData::getOrdinal() const
{
	return 0;
}

void TerrainLayerComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

}

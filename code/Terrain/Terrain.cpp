/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Terrain.h"

#include "Heightfield/Heightfield.h"
#include "Render/ITexture.h"
#include "Resource/IResourceManager.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.Terrain", Terrain, Object)

bool Terrain::bindTile(resource::IResourceManager* resourceManager, int32_t tile, TileResources& outResources) const
{
	if (tile < 0 || tile >= (int32_t)m_tiles.size())
		return false;

	return bindResources(resourceManager, m_tiles[tile], outResources);
}

bool Terrain::bindResources(resource::IResourceManager* resourceManager, const TerrainResource::Tile& tile, TileResources& outResources)
{
	if (!resourceManager->bind(tile.heightfield, outResources.heightfield))
		return false;
	if (!resourceManager->bind(tile.heightMap, outResources.heightMap))
		return false;
	if (!resourceManager->bind(tile.normalMap, outResources.normalMap))
		return false;
	if (!resourceManager->bind(tile.cutMap, outResources.cutMap))
		return false;
	if (!resourceManager->bind(tile.splatMap, outResources.splatMap))
		return false;
	if (tile.colorMap && !resourceManager->bind(tile.colorMap, outResources.colorMap))
		return false;
	return true;
}

}

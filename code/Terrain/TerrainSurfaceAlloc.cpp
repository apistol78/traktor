/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Terrain/TerrainSurfaceAlloc.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceAlloc", TerrainSurfaceAlloc, Object)

TerrainSurfaceAlloc::TerrainSurfaceAlloc(uint32_t size)
:	m_size(size)
{
	m_tileDims[0] = std::max< uint32_t >(size / 4, 1);
	m_tileDims[1] = std::max< uint32_t >(size / 8, 1);
	m_tileDims[2] = std::max< uint32_t >(size / 16, 1);
	m_tileDims[3] = std::max< uint32_t >(size / 64, 1);

	for (uint32_t i = 0; i < sizeof_array(m_tileDims); ++i)
	{
		const uint32_t dim = m_tileDims[i];
		const uint32_t rows = m_tileDims[0] / dim;
		for (uint32_t row = 0; row < rows; ++row)
		{
			const uint32_t columns = size / dim;
			for (uint32_t col = 0; col < columns; ++col)
			{
				Tile& t = m_free[i].push_back();
				t.lod = i;
				t.x = col * dim;
				t.y = row * dim + i * m_tileDims[0];
				t.dim = dim;
			}
		}
	}
}

bool TerrainSurfaceAlloc::alloc(uint32_t lod, Tile& outTile)
{
	if (m_free[lod].empty())
	{
		outTile.dim = 0;
		return false;
	}

	outTile = m_free[lod].back();
	m_free[lod].pop_back();

	T_FATAL_ASSERT(outTile.lod == lod);
	return true;
}

void TerrainSurfaceAlloc::free(const Tile& tile)
{
	m_free[tile.lod].push_back(tile);
}

}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Terrain/TerrainSurfaceAlloc.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const uint32_t c_tileDims[] = { 512, 512, 256, 128 };

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceAlloc", TerrainSurfaceAlloc, Object)

TerrainSurfaceAlloc::TerrainSurfaceAlloc()
{
	for (uint32_t i = 0; i < sizeof_array(c_tileDims); ++i)
	{
		uint32_t dim = c_tileDims[i];
		uint32_t rows = 1024 / dim;
		for (uint32_t row = 0; row < rows; ++row)
		{
			uint32_t columns = 4096 / dim;
			for (uint32_t col = 0; col < columns; ++col)
			{
				Tile t;
				t.size = i;
				t.x = col * dim;
				t.y = row * dim + i * 1024;
				t.dim = dim;
				m_free[i].push_back(t);
			}
		}
	}
}

bool TerrainSurfaceAlloc::alloc(uint32_t size, Tile& outTile)
{
	if (m_free[size].empty())
		return false;

	outTile = m_free[size].back();
	m_free[size].pop_back();

	return true;
}

void TerrainSurfaceAlloc::free(const Tile& tile)
{
	m_free[tile.size].push_back(tile);
}

	}
}

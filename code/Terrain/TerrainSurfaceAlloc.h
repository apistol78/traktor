#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor::terrain
{

/*! Terrain surface allocator.
 * \ingroup Terrain
 */
class TerrainSurfaceAlloc : public Object
{
	T_RTTI_CLASS;

public:
	struct Tile
	{
		uint32_t lod = 0;
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t dim = 0;
	};

	TerrainSurfaceAlloc(uint32_t size);

	/*! Alloc tile.
	 *
	 * \param lod Tile lod, 0-3.
	 */
	bool alloc(uint32_t lod, Tile& outTile);

	void free(const Tile& tile);

	uint32_t getVirtualSize() const { return m_size; }

	uint32_t getTileDim(uint32_t lod) const { return m_tileDims[lod]; }

private:
	uint32_t m_size;
	uint32_t m_tileDims[4];
	AlignedVector< Tile > m_free[4];
};

}

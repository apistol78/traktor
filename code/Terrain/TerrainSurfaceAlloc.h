#pragma once

#include <list>
#include "Core/Object.h"

namespace traktor
{
	namespace terrain
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
		uint32_t size;
		uint32_t x;
		uint32_t y;
		uint32_t dim;

		Tile()
		:	size(0)
		,	x(0)
		,	y(0)
		,	dim(0)
		{
		}
	};

	TerrainSurfaceAlloc();

	/*! Alloc tile.
	 *
	 * \param size Tile size, 0 = 1024, 1 = 512, 2 = 256, 3 = 128
	 */
	bool alloc(uint32_t size, Tile& outTile);

	void free(const Tile& tile);

private:
	std::list< Tile > m_free[5];
};

	}
}


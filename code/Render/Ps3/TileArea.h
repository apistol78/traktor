#ifndef traktor_render_TileArea_H
#define traktor_render_TileArea_H

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

class TileArea
{
public:
	struct TileInfo
	{
		uint32_t index;
		uint32_t base;
		uint32_t size;
		uint32_t dramBank;

		TileInfo()
		:	index(~0UL)
		,	base(~0UL)
		,	size(~0UL)
		,	dramBank(~0UL)
		{
		}
	};
	
	TileArea(uint32_t areaCount, uint32_t tagSize);
	
	bool alloc(uint32_t size, uint32_t alignment, TileInfo& outTileInfo);
	
	void free(uint32_t index);
	
private:
	uint32_t m_areaCount;
	uint32_t m_tagSize;
	TileInfo m_tiles[32];
	uint32_t m_allocated;
	uint32_t m_tagTop;
	uint32_t m_dramBank;
};

	}
}

#endif	// traktor_render_TileArea_H

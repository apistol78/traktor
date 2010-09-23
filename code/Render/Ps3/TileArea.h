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
		uint32_t tagBase;
		uint32_t tagSize;
		uint32_t dramBank;
	};
	
	TileArea();
	
	bool alloc(uint32_t size, TileInfo& outTileInfo);
	
	void free(uint32_t index);
	
private:
	TileInfo m_tiles[15];
	uint32_t m_allocated;
	uint32_t m_tagTop;
	uint32_t m_dramBank;
};

	}
}

#endif	// traktor_render_TileArea_H

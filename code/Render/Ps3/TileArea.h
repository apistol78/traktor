/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TileArea_H
#define traktor_render_TileArea_H

#include "Core/Config.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Ps3/BlockList.h"

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
	Semaphore m_lock;
	uint32_t m_areaCount;
	TileInfo m_tiles[32];
	uint32_t m_allocated;
	uint32_t m_dramBank;
	BlockList m_tag;
};

	}
}

#endif	// traktor_render_TileArea_H

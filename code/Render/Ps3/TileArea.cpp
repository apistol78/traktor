/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Acquire.h"
#include "Render/Ps3/TileArea.h"

namespace traktor
{
	namespace render
	{

TileArea::TileArea(uint32_t areaCount, uint32_t tagSize)
:	m_areaCount(areaCount)
,	m_allocated(0)
,	m_dramBank(0)
,	m_tag(tagSize)
{
	for (uint32_t i = 0; i < sizeof_array(m_tiles); ++i)
	{
		m_tiles[i].index = i;
		m_tiles[i].base = 0;
		m_tiles[i].size = 0;
		m_tiles[i].dramBank = 0;
	}
}

bool TileArea::alloc(uint32_t size, uint32_t alignment, TileInfo& outTileInfo)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (outTileInfo.index == ~0UL);

	uint32_t base = m_tag.alloc(size, alignment);
	if (base == BlockList::NotEnoughSpace)
	{
		log::error << L"Unable to allocate tile area; out of tag space" << Endl;
		return false;
	}
	
	uint32_t index = ~0UL;	
	for (uint32_t i = 0; i < m_areaCount; ++i)
	{
		if ((m_allocated & (1 << i)) == 0)
		{
			index = i;
			break;
		}
	}
	if (index == ~0UL)
	{
		log::error << L"Unable to allocate tile area; out of tiles" << Endl;
		return false;
	}

	m_allocated |= 1 << index;
	
	m_tiles[index].base = base; 
	m_tiles[index].size = size;
	m_tiles[index].dramBank = m_dramBank++ % 4;
	
	outTileInfo = m_tiles[index];
	return true;
}

void TileArea::free(uint32_t index)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_allocated & (1 << index))
	{
		m_tag.free(m_tiles[index].base);
		
		m_tiles[index].base = 0;
		m_tiles[index].size = 0;
		m_tiles[index].dramBank = 0;
		
		m_allocated &= ~(1 << index);
	}
}

	}
}

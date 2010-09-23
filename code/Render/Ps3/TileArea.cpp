#include "Core/Log/Log.h"
#include "Render/Ps3/TileArea.h"

namespace traktor
{
	namespace render
	{

TileArea::TileArea()
:	m_allocated(0)
,	m_tagTop(0)
,	m_dramBank(0)
{
	for (uint32_t i = 0; i < sizeof_array(m_tiles); ++i)
	{
		m_tiles[i].index = i;
		m_tiles[i].tagBase = 0;
		m_tiles[i].tagSize = 0;
		m_tiles[i].dramBank = 0;
	}
}

bool TileArea::alloc(uint32_t size, TileInfo& outTileInfo)
{
	uint32_t tagSize = size / 0x10000;
	
	if (tagSize > 0x7ff - m_tagTop)
	{
		log::error << L"Unable to allocate tile area; out of compression space" << Endl;
		return false;
	}
	
	uint32_t index = ~0UL;	
	for (uint32_t i = 0; i < sizeof_array(m_tiles); ++i)
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
	
	m_tiles[index].tagBase = m_tagTop; 
	m_tiles[index].tagSize = tagSize;
	m_tiles[index].dramBank = m_dramBank++ % 4;
	
	m_tagTop += tagSize;
	
	outTileInfo = m_tiles[index];
	return true;
}

void TileArea::free(uint32_t index)
{
	T_ASSERT (m_allocated & (1 << index));
	
	if (m_tiles[index].tagBase + m_tiles[index].tagSize == m_tagTop)
		m_tagTop = m_tiles[index].tagBase;
	
	m_tiles[index].tagBase = 0;
	m_tiles[index].tagSize = 0;
	m_tiles[index].dramBank = 0;
	
	m_allocated &= ~(1 << index);
}

	}
}

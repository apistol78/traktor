#include "Core/Misc/Align.h"
#include "Flash/Acc/BlockList.h"

namespace traktor
{
	namespace flash
	{

BlockList::BlockList(uint32_t size)
:	m_size(size)
{
}

uint32_t BlockList::alloc(uint32_t size, uint32_t alignment)
{
	if (m_blocks.empty())
	{
		Block blk;
		blk.offset = 0;
		blk.size = size;
		m_blocks.push_back(blk);
		return 0;
	}
	
	uint32_t offset = 0;
	
	for (std::list< Block >::iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		if ((i->offset - offset) >= size)
		{
			Block blk;
			blk.offset = offset;
			blk.size = size;
			m_blocks.insert(i, blk);
			return offset;
		}
		offset = alignUp(i->offset + i->size, alignment);
	}

	if ((m_size - offset) >= size)
	{
		Block blk;
		blk.offset = offset;
		blk.size = size;
		m_blocks.push_back(blk);
		return offset;
	}
	
	return NotEnoughSpace;
}

void BlockList::free(uint32_t offset)
{
	for (std::list< Block >::iterator i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		if (offset == i->offset)
		{
			m_blocks.erase(i);
			return;
		}
	}
	T_FATAL_ERROR;
}

	}
}

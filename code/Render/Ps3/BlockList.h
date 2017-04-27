/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_BlockList_H
#define traktor_render_BlockList_H

#include <list>
#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

class BlockList
{
public:
	enum { NotEnoughSpace = ~0UL };
		
	BlockList(uint32_t size);

	uint32_t alloc(uint32_t size, uint32_t alignment);

	void free(uint32_t offset);

private:
	struct Block
	{
		uint32_t offset;
		uint32_t size;
	};
	
	uint32_t m_size;
	std::list< Block > m_blocks;
};

	}
}

#endif	// traktor_render_BlockList_H

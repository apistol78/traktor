#ifndef traktor_flash_BlockList_H
#define traktor_flash_BlockList_H

#include <list>
#include "Core/Config.h"

namespace traktor
{
	namespace flash
	{

class BlockList
{
public:
	enum { NotEnoughSpace = ~0U };

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

#endif	// traktor_flash_BlockList_H

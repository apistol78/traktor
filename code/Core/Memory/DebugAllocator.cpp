#include "Core/Memory/DebugAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

const size_t c_wallSize = 16;
const size_t c_maxFreedBlocks = 1000;

	}

DebugAllocator::DebugAllocator(IAllocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
{
}

DebugAllocator::~DebugAllocator()
{
	T_FATAL_ASSERT (m_aliveBlocks.empty());
	
	for (std::list< Block >::iterator i = m_freedBlocks.begin(); i != m_freedBlocks.end(); ++i)
		m_systemAllocator->free(i->top);
}

void* DebugAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);

	checkBlocks();

	uint8_t* ptr = static_cast< uint8_t* >(m_systemAllocator->alloc(size + c_wallSize * 2, align, tag));
	if (!ptr)
		return 0;

	for (size_t i = 0; i < c_wallSize; ++i)
	{
		ptr[i] = 0xaa;
		ptr[i + c_wallSize + size] = 0x55;
	}

	m_aliveBlocks.push_front(Block());
	m_aliveBlocks.front().top = ptr;
	m_aliveBlocks.front().size = size + c_wallSize * 2;

	return ptr + c_wallSize;	
}

void DebugAllocator::free(void* ptr)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);

	checkBlocks();

	if (!ptr)
		return;

	for (std::list< Block >::iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		if (static_cast< uint8_t* >(i->top) + c_wallSize == ptr)
		{
			uint8_t* bptr = static_cast< uint8_t* >(i->top);
			for (size_t j = 0; j < i->size; ++j)
				bptr[j] = 0xdd;

			m_freedBlocks.push_back(*i);
			m_aliveBlocks.erase(i);
			return;
		}
	}

	T_FATAL_ERROR;
}

void DebugAllocator::checkBlocks()
{
	// Check walls on alive blocks.
	for (std::list< Block >::iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		uint8_t* ptr = static_cast< uint8_t* >(i->top);
		for (size_t j = 0; j < c_wallSize; ++j)
			T_FATAL_ASSERT (ptr[j] == 0xaa);
		for (size_t j = i->size - c_wallSize; j < i->size; ++j)
			T_FATAL_ASSERT (ptr[j] == 0x55);
	}

	// Ensure freed blocks aren't modified after they have been released.
	for (std::list< Block >::iterator i = m_freedBlocks.begin(); i != m_freedBlocks.end(); ++i)
	{
		uint8_t* ptr = static_cast< uint8_t* >(i->top);
		for (size_t j = 0; j < i->size; ++j)
			T_FATAL_ASSERT (ptr[j] == 0xdd);
	}

	// Reduce load; we cannot keep every released block forever.
	while (m_freedBlocks.size() > c_maxFreedBlocks)
	{
		m_systemAllocator->free(m_freedBlocks.front().top);
		m_freedBlocks.pop_front();
	}
}

}

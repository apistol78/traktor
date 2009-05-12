#include "Core/Heap/DebugAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

const size_t c_wallSize = 8;

	}

DebugAllocator::DebugAllocator(Allocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
{
}

DebugAllocator::~DebugAllocator()
{
	T_ASSERT (m_aliveBlocks.empty());
	
	for (std::list< Block >::iterator i = m_freedBlocks.begin(); i != m_freedBlocks.end(); ++i)
		m_systemAllocator->free(i->top);

	delete m_systemAllocator;
}

void* DebugAllocator::alloc(size_t size, size_t align)
{
	Acquire< CriticalSection > scope(m_lock);

	checkBlocks();

	uint8_t* ptr = static_cast< uint8_t* >(m_systemAllocator->alloc(size + c_wallSize * 2, align));
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
	Acquire< CriticalSection > scope(m_lock);

	checkBlocks();

	if (!ptr)
		return;

	for (std::list< Block >::iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		if (static_cast< uint8_t* >(i->top) + c_wallSize == ptr)
		{
			uint8_t* ptr = static_cast< uint8_t* >(i->top);
			for (size_t j = 0; j < i->size; ++j)
				ptr[j] = 0xdd;

			m_freedBlocks.push_back(*i);
			m_aliveBlocks.erase(i);
			return;
		}
	}

	T_ASSERT (0);
}

void DebugAllocator::checkBlocks()
{
	// Check walls on alive blocks.
	for (std::list< Block >::iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		uint8_t* ptr = static_cast< uint8_t* >(i->top);
		for (size_t j = 0; j < c_wallSize; ++j)
			T_ASSERT (ptr[j] == 0xaa);
		for (size_t j = i->size - c_wallSize; j < i->size; ++j)
			T_ASSERT (ptr[j] == 0x55);
	}

	// Ensure freed blocks aren't modified after they have been released.
	for (std::list< Block >::iterator i = m_freedBlocks.begin(); i != m_freedBlocks.end(); ++i)
	{
		uint8_t* ptr = static_cast< uint8_t* >(i->top);
		for (size_t j = 0; j < i->size; ++j)
			T_ASSERT (ptr[j] == 0xdd);
	}
}

Allocator::MemoryType DebugAllocator::type(void* ptr) const
{
	Acquire< CriticalSection > scope(m_lock);

	for (std::list< Block >::const_iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		uint8_t* top = static_cast< uint8_t* >(i->top);
		if (ptr >= top && ptr < top + i->size)
		{
			if (ptr >= top + c_wallSize && ptr < top + i->size - c_wallSize)
				return MtAllocated;
			else
				return MtDebug;
		}
	}

	for (std::list< Block >::const_iterator i = m_freedBlocks.begin(); i != m_freedBlocks.end(); ++i)
	{
		uint8_t* top = static_cast< uint8_t* >(i->top);
		if (ptr >= top && ptr < top + i->size)
		{
			if (ptr >= top + c_wallSize && ptr < top + i->size - c_wallSize)
				return MtFreed;
			else
				return MtDebug;
		}
	}

	return MtUnknown;
}

}

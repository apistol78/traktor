/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Debug/CallStack.h"
#include "Core/Memory/DebugAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

const uint32_t c_wallHead = 0xaaaaaaaa;
const uint32_t c_wallTail = 0x55555555;
const uint32_t c_wallDead = 0xdddddddd;
const size_t c_wallSize = 16;
const size_t c_maxFreedBlocks = 32;
const int32_t c_eventsUntilCheck = 128;

	}

DebugAllocator::DebugAllocator(IAllocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
,	m_untilCheck(c_eventsUntilCheck)
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

	uint32_t* whp = reinterpret_cast< uint32_t* >(ptr);
	uint32_t* wtp = reinterpret_cast< uint32_t* >(ptr + c_wallSize + size);
	for (size_t i = 0; i < c_wallSize / sizeof(uint32_t); ++i)
	{
		*whp++ = c_wallHead;
		*wtp++ = c_wallTail;
	}

	m_aliveBlocks.push_front(Block());

	Block& block = m_aliveBlocks.front();

	block.top = ptr;
	block.size = size + c_wallSize * 2;

	for (int i = 0; i < sizeof_array(block.at); ++i)
		block.at[i] = 0;

	getCallStack(sizeof_array(block.at), block.at, 1);

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
			uint32_t* whp = reinterpret_cast< uint32_t* >(i->top);
			uint32_t* wtp = reinterpret_cast< uint32_t* >(i->top + i->size - c_wallSize);
			for (size_t i = 0; i < c_wallSize / sizeof(uint32_t); ++i)
			{
				*whp++ = c_wallDead;
				*wtp++ = c_wallDead;
			}

			m_freedBlocks.push_back(*i);
			m_aliveBlocks.erase(i);
			return;
		}
	}

	T_FATAL_ERROR;
}

void DebugAllocator::checkBlocks()
{
	// Skip checks; else application will be too slow.
	if (--m_untilCheck > 0)
		return;

	m_untilCheck = c_eventsUntilCheck;

	// Check walls on alive blocks.
	for (std::list< Block >::iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		uint32_t* whp = reinterpret_cast< uint32_t* >(i->top);
		uint32_t* wtp = reinterpret_cast< uint32_t* >(i->top + i->size - c_wallSize);
		for (size_t j = 0; j < c_wallSize / sizeof(uint32_t); ++j)
		{
			T_FATAL_ASSERT (whp[j] == c_wallHead);
			T_FATAL_ASSERT (wtp[j] == c_wallTail);
		}
	}

	// Ensure freed blocks aren't modified after they have been released.
	for (std::list< Block >::iterator i = m_freedBlocks.begin(); i != m_freedBlocks.end(); ++i)
	{
		uint32_t* whp = reinterpret_cast< uint32_t* >(i->top);
		uint32_t* wtp = reinterpret_cast< uint32_t* >(i->top + i->size - c_wallSize);
		for (size_t j = 0; j < c_wallSize / sizeof(uint32_t); ++j)
		{
			T_FATAL_ASSERT (whp[j] == c_wallDead);
			T_FATAL_ASSERT (wtp[j] == c_wallDead);
		}
	}

	// Reduce load; we cannot keep every released block forever.
	while (m_freedBlocks.size() > c_maxFreedBlocks)
	{
		m_systemAllocator->free(m_freedBlocks.front().top);
		m_freedBlocks.pop_front();
	}
}

}

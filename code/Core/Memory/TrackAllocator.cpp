#include <iostream>
#if !defined(_WIN32) && !defined(_PS3) && !defined(__ANDROID__)
#	include <execinfo.h>
#endif
#include "Core/Platform.h"
#include "Core/Debug/CallStack.h"
#include "Core/Memory/TrackAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{

TrackAllocator::TrackAllocator(IAllocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
{
}

TrackAllocator::~TrackAllocator()
{
	wchar_t buf[512];

#if defined(_WIN32)
	wsprintf(buf, L"\nAllocation stats:\n");
	OutputDebugString(buf);

	for (std::map< void*, Stats >::const_iterator i = m_allocStats.begin(); i != m_allocStats.end(); ++i)
	{
		wsprintf(buf, L"0x%p, %d time(s) totally %d byte(s), tag \"%S\"\n", i->first, i->second.count, i->second.memory, i->second.tag);
		OutputDebugString(buf);
	}
#endif

	if (!m_aliveBlocks.empty())
	{
		std::map< Block, uint32_t > frequency;

#if defined(_WIN32)
		wsprintf(buf, L"\nMemory leak detected, following %d allocation(s) not freed:\n", m_aliveBlocks.size());
		OutputDebugString(buf);

		for (std::map< void*, Block >::const_iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
		{
			wsprintf(buf, L"0x%p, %d byte(s), tag \"%S\"\n", i->first, i->second.size, i->second.tag);
			OutputDebugString(buf);
			for (int j = 0; j < sizeof_array(i->second.at); ++j)
			{
				wsprintf(buf, L"   %d: 0x%p\n", j, i->second.at[j]);
				OutputDebugString(buf);
			}
			frequency[i->second]++;
		}

		OutputDebugString(L"\nLeak Path Frequency:\n");

		for (std::map< Block, uint32_t >::const_iterator i = frequency.begin(); i != frequency.end(); ++i)
		{
			wsprintf(buf, L"0x%p: %d allocation(s)\n", i->first.at[0], i->second);
			OutputDebugString(buf);

			for (int j = 0; j < sizeof_array(i->first.at); ++j)
			{
				wsprintf(buf, L"   %d: 0x%p\n", j, i->first.at[j]);
				OutputDebugString(buf);
			}
		}
#else
		std::wcout << L"Memory leak detected, following allocation(s) not freed:" << std::endl;
		for (std::map< void*, Block >::const_iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
		{
			std::wcout << L"0x" << i->first << L", " << i->second.size << L" byte(s), tag \"" << i->second.tag << L"\"" << std::endl;
			for (int j = 0; j < sizeof_array(i->second.at); ++j)
				std::wcout << L"   " << j << L": 0x" << i->second.at[j] << std::endl;
		}
#endif
	}
	else
		std::wcout << L"No memory leaks! Good work!" << std::endl;
}

void* TrackAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	void* ptr = m_systemAllocator->alloc(size, align, tag);
	if (!ptr)
		return 0;

	Block block;
	block.tag = tag;
	block.size = size;

	for (int i = 0; i < sizeof_array(block.at); ++i)
		block.at[i] = 0;

	getCallStack(sizeof_array(block.at), block.at, 1);

	m_aliveBlocks.insert(std::make_pair(ptr, block));
	
	m_allocStats[block.at[0]].tag = tag;
	m_allocStats[block.at[0]].count++;
	m_allocStats[block.at[0]].memory += size;

	return ptr;
}

void TrackAllocator::free(void* ptr)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< void*, Block >::iterator i = m_aliveBlocks.find(ptr);
	if (i != m_aliveBlocks.end())
		m_aliveBlocks.erase(i);

	m_systemAllocator->free(ptr);
}

bool TrackAllocator::Block::operator < (const Block& rh) const
{
	for (int32_t i = 0; i < sizeof_array(at); ++i)
	{
		if (at[i] < rh.at[i])
			return true;
		if (at[i] > rh.at[i])
			return false;
	}
	return false;
}

}

#include <iostream>
#if !defined(_WIN32) && !defined(_PS3)
#	include <execinfo.h>
#endif
#include "Core/Platform.h"
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
	if (!m_aliveBlocks.empty())
	{
		std::map< void*, uint32_t > frequency;

#if defined(_WIN32)
		wchar_t buf[512];
		wsprintf(buf, L"Memory leak detected, following %d allocation(s) not freed:\n", m_aliveBlocks.size());
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
			frequency[i->second.at[0]]++;
		}

		for (std::map< void*, uint32_t >::const_iterator i = frequency.begin(); i != frequency.end(); ++i)
		{
			wsprintf(buf, L"0x%p: %d allocation(s)\n", i->first, i->second);
			OutputDebugString(buf);
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
	block.at[0] =
	block.at[1] =
	block.at[2] =
	block.at[3] = 0;

#if defined(_WIN32)

	CaptureStackBackTrace(
		2,
		sizeof_array(block.at),
		block.at,
		0
	);

#elif !defined(_PS3)

	backtrace(
		block.at,
		sizeof_array(block.at)
	);

#endif

	m_aliveBlocks.insert(std::make_pair(ptr, block));
	m_allocCount[block.at[0]]++;

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

}

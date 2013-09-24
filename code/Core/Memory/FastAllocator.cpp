#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/SystemConstruct.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{
	namespace
	{

const uint32_t c_blockCounts[] =
{
#if defined(WINCE)
	1024,
	1024,
	1024,
	1024,
	1024,
#elif defined(_PS3)
	32768,				// 16
	36864,				// 32
	20480,				// 64
	8192,				// 128
	4096				// 256
#else
	131072,				// 16
	131072,				// 32
	40960,				// 64
	16384,				// 128
	16384				// 256
#endif
};

	}

FastAllocator::FastAllocator(IAllocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
{
	for (size_t i = 0; i < sizeof_array(c_blockCounts); ++i)
	{
		uint32_t qsize = 1UL << (i + 4);
		m_blockAlloc[i] = allocConstruct< BlockAllocator >(
			m_systemAllocator->alloc(qsize * c_blockCounts[i], 16, T_FILE_LINE),
			c_blockCounts[i],
			qsize
		);
		m_blockAllocLock[i] = 0;
		m_blockAllocFull[i] = 0;
	}
}

FastAllocator::~FastAllocator()
{
	for (size_t i = 0; i < sizeof_array(m_blockAlloc); ++i)
	{
		m_systemAllocator->free(m_blockAlloc[i]->top());
		freeDestruct(m_blockAlloc[i]);
	}
}

void* FastAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	void* p = 0;

	if (size > 0 && size <= 256 && align <= 16)
	{
		if (size < 16)
			size = 16;

		size = nearestLog2(size);

		uint32_t qid = log2(size);
		uint32_t qsize = 1UL << qid;

		qid -= 4;

		BlockAllocator* blockAlloc = m_blockAlloc[qid];
		T_ASSERT (blockAlloc)

		{
			while (Atomic::exchange(m_blockAllocLock[qid], 1) != 0)
				ThreadManager::getInstance().getCurrentThread()->yield();

			p = blockAlloc->alloc();

			Atomic::exchange(m_blockAllocLock[qid], 0);
		}

		if (!p)
		{
#if 0
			if (!m_blockAllocFull[qid])
				log::debug << L"Out of " << size << L" blocks in fast allocator" << Endl;
#endif
			m_blockAllocFull[qid] = 1;
		}

		T_ASSERT (alignUp((uint8_t*)p, 16) == p);
	}
	
	if (!p)
		p = m_systemAllocator->alloc(size, align, tag);
	
	T_ASSERT (alignUp((uint8_t*)p, align) == p);
	return p;
}

void FastAllocator::free(void* ptr)
{
	for (size_t i = 0; i < sizeof_array(m_blockAlloc); ++i)
	{
		if (m_blockAlloc[i]->belong(ptr))
		{
			{
				while (Atomic::exchange(m_blockAllocLock[i], 1) != 0)
					ThreadManager::getInstance().getCurrentThread()->yield();

				m_blockAlloc[i]->free(ptr);

				Atomic::exchange(m_blockAllocLock[i], 0);
			}
			m_blockAllocFull[i] = 0;
			return;
		}
	}
	m_systemAllocator->free(ptr);
}

}

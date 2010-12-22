#include "Core/Math/Log2.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/SystemConstruct.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Acquire.h"

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
	32768,				// 16
	36864,				// 32
	20480,				// 64
	8192,				// 128
	4096				// 256
#endif
};

	}

FastAllocator::FastAllocator(IAllocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
{
	for (size_t i = 0; i < sizeof_array(m_blockAlloc); ++i)
		m_blockAlloc[i] = 0;
}

FastAllocator::~FastAllocator()
{
	for (size_t i = 0; i < sizeof_array(m_blockAlloc); ++i)
	{
		if (!m_blockAlloc[i])
			continue;
		m_systemAllocator->free(m_blockAlloc[i]->top());
		freeDestruct(m_blockAlloc[i]);
	}
}

void* FastAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	void* p = 0;

	if (size > 0 && size <= 256 && align <= 16)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		if (size < 16)
			size = 16;

		size = nearestLog2(size);

		size_t qid = log2(size);
		size_t qsize = 1 << qid;

		qid -= 4;

		BlockAllocator* blockAlloc = m_blockAlloc[qid];
		if (!blockAlloc)
			m_blockAlloc[qid] = blockAlloc = allocConstruct< BlockAllocator >(
				m_systemAllocator->alloc(qsize * c_blockCounts[qid], 16, T_FILE_LINE),
				c_blockCounts[qid],
				qsize
			);

		p = blockAlloc->alloc();
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
		if (m_blockAlloc[i] && m_blockAlloc[i]->belong(ptr))
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_blockAlloc[i]->free(ptr);
			return;
		}
	}
	m_systemAllocator->free(ptr);
}

}

#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

#if !defined(WINCE)
const uint32_t c_blockCount = 8192;		// (256 + 128 + 64 + 32 + 16) * 8192 = 4063232 ~4 Mb
#else
const uint32_t c_blockCount = 2048;		// (256 + 128 + 64 + 32 + 16) * 2048 = 1015808 ~1 Mb
#endif

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
		delete m_blockAlloc[i];
	}
}

void* FastAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	void* p = 0;
	
	if (size <= 256)
	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);

		size_t qsize = (size + 15) & ~15UL;
		size_t qid = (qsize >> 4) - 1;
		T_ASSERT (qid < sizeof_array(m_blockAlloc));

		BlockAllocator* blockAlloc = m_blockAlloc[qid];
		if (!blockAlloc)
			m_blockAlloc[qid] = blockAlloc = new BlockAllocator(
				m_systemAllocator->alloc(qsize * c_blockCount, 16, tag),
				c_blockCount,
				qsize
			);

		p = blockAlloc->alloc();
	}
	
	if (!p)
		p = m_systemAllocator->alloc(size, align, tag);
		
	return p;
}

void FastAllocator::free(void* ptr)
{
	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);
		for (size_t i = 0; i < sizeof_array(m_blockAlloc); ++i)
		{
			if (m_blockAlloc[i] && m_blockAlloc[i]->free(ptr))
				return;
		}
	}	
	m_systemAllocator->free(ptr);
}

}

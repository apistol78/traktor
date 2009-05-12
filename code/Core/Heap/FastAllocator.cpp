#include "Core/Heap/FastAllocator.h"
#include "Core/Heap/BlockAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

const uint32_t c_blockCount = 8192;

	}

FastAllocator::FastAllocator(Allocator* systemAllocator)
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
	delete m_systemAllocator;
}

void* FastAllocator::alloc(size_t size, size_t align)
{
	void* p = 0;
	
	if (size <= 256)
	{
		Acquire< CriticalSection > scope(m_lock);

		size_t qsize = (size + 15) & ~15UL;
		size_t qid = (qsize >> 4) - 1;
		T_ASSERT (qid < sizeof_array(m_blockAlloc));

		BlockAllocator* blockAlloc = m_blockAlloc[qid];
		if (!blockAlloc)
			m_blockAlloc[qid] = blockAlloc = new BlockAllocator(
				m_systemAllocator->alloc(qsize * c_blockCount, 16),
				c_blockCount,
				qsize
			);

		p = blockAlloc->alloc();
	}
	
	if (!p)
		p = m_systemAllocator->alloc(size, align);
		
	return p;
}

void FastAllocator::free(void* ptr)
{
	{
		Acquire< CriticalSection > scope(m_lock);
		for (size_t i = 0; i < sizeof_array(m_blockAlloc); ++i)
		{
			if (m_blockAlloc[i] && m_blockAlloc[i]->free(ptr))
				return;
		}
	}	
	m_systemAllocator->free(ptr);
}

Allocator::MemoryType FastAllocator::type(void* ptr) const
{
	return m_systemAllocator->type(ptr);
}

}

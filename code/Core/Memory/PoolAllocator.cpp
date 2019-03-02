#include <cstring>
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Misc/Align.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PoolAllocator", PoolAllocator, Object)

PoolAllocator::PoolAllocator()
:	m_totalSize(0)
,	m_head(0)
,	m_tail(0)
{
}

PoolAllocator::PoolAllocator(IAllocator* allocator, uint32_t totalSize)
:	m_allocator(allocator)
,	m_totalSize(totalSize)
,	m_head(0)
,	m_tail(0)
{
}

PoolAllocator::PoolAllocator(uint32_t totalSize)
:	m_allocator(new StdAllocator())
,	m_totalSize(totalSize)
,	m_head(0)
,	m_tail(0)
{
}

PoolAllocator::PoolAllocator(void* heap, uint32_t totalSize)
:	m_totalSize(totalSize)
,	m_head(static_cast< uint8_t* >(heap))
,	m_tail(static_cast< uint8_t* >(heap))
{
	T_ASSERT (m_head);
}

PoolAllocator::~PoolAllocator()
{
	T_EXCEPTION_GUARD_BEGIN;

	if (m_allocator)
	{
		for (std::list< uint8_t* >::iterator i = m_heaps.begin(); i != m_heaps.end(); ++i)
			m_allocator->free(*i);
	}

	T_EXCEPTION_GUARD_END;
}

void PoolAllocator::enter()
{
	m_scope.push(m_tail);
}

void PoolAllocator::leave()
{
	T_ASSERT (!m_scope.empty());

	uint8_t* tail = m_scope.top();
	m_scope.pop();

	// Free exceeding heaps.
	if (m_allocator && !(tail >= m_head && tail <= m_head + m_totalSize))
	{
		m_head = 0;

		// Find heap in which the tail is stored.
		std::list< uint8_t* >::iterator i = m_heaps.begin();
		for ( ;i != m_heaps.end(); ++i)
		{
			if (tail >= (*i) && tail <= (*i) + m_totalSize)
				break;
		}
		T_ASSERT (i != m_heaps.end());

		m_head = *i;

		// Free heaps beyond pop;ed tail.
		for (std::list< uint8_t* >::iterator j = ++i; j != m_heaps.end(); ++j)
			m_allocator->free(*j);

		m_heaps.erase(i, m_heaps.end());
	}

	m_tail = tail;
}

void* PoolAllocator::alloc(uint32_t size, uint32_t align)
{
	T_ASSERT (size <= m_totalSize);

	if (!m_head || uint32_t(m_tail - m_head) + (size + align - 1) >= m_totalSize)
	{
		if (m_allocator)
		{
			// Allocate new heap.
			uint8_t* heap = (uint8_t*)m_allocator->alloc(m_totalSize, 16, T_FILE_LINE);
			T_FATAL_ASSERT_M (heap, L"Out of memory (pool)");

			m_head =
			m_tail = heap;

			m_heaps.push_back(heap);
		}
		else
			T_FATAL_ERROR;
	}

	uint8_t* ptr = (uint8_t*)m_tail;
	ptr = alignUp(ptr, align);

	m_tail = ptr + size;
	return ptr;
}

}

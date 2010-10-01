#include <cstring>
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/StdAllocator.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PoolAllocator", PoolAllocator, Object)

PoolAllocator::PoolAllocator()
:	m_allocator(0)
,	m_totalSize(0)
,	m_head(0)
,	m_tail(0)
{
}

PoolAllocator::PoolAllocator(IAllocator* allocator, uint32_t totalSize)
:	m_allocator(allocator)
,	m_totalSize(totalSize)
{
	m_head = static_cast< uint8_t* >(m_allocator->alloc(totalSize, 16, T_FILE_LINE));
	m_tail = m_head;
	T_ASSERT (m_head);
}

PoolAllocator::PoolAllocator(uint32_t totalSize)
:	m_allocator(new StdAllocator())
,	m_totalSize(totalSize)
{
	m_head = static_cast< uint8_t* >(m_allocator->alloc(totalSize, 16, T_FILE_LINE));
	m_tail = m_head;
	T_ASSERT (m_head);
}

PoolAllocator::PoolAllocator(void* heap, uint32_t totalSize)
:	m_allocator(0)
,	m_totalSize(totalSize)
,	m_head(static_cast< uint8_t* >(heap))
,	m_tail(static_cast< uint8_t* >(heap))
{
	T_ASSERT (m_head);
}

PoolAllocator::~PoolAllocator()
{
	T_EXCEPTION_GUARD_BEGIN;

	if (m_allocator)
		m_allocator->free(m_head);

	T_EXCEPTION_GUARD_END;
}

void PoolAllocator::enter()
{
	m_scope.push(m_tail);
}

void PoolAllocator::leave()
{
	T_ASSERT (!m_scope.empty());
	m_tail = m_scope.top();
	m_scope.pop();
}

void* PoolAllocator::alloc(uint32_t size)
{
	T_ASSERT_M (uint32_t(m_tail - m_head) + size < m_totalSize, L"Out of memory (pool)");
	void* ptr = m_tail;
	m_tail += size;
	return ptr;
}

}

#include <cstring>
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/StdAllocator.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PoolAllocator", PoolAllocator, Object)

PoolAllocator::PoolAllocator(IAllocator* allocator, uint32_t totalSize)
:	m_allocator(allocator)
,	m_totalSize(totalSize)
{
	m_head = static_cast< uint8_t* >(m_allocator->alloc(totalSize, 16, L"Pool"));
	m_tail = m_head;

	T_ASSERT (m_head);

	std::memset(m_head, 0, totalSize);
}

PoolAllocator::PoolAllocator(uint32_t totalSize)
:	m_allocator(new StdAllocator())
,	m_totalSize(totalSize)
{
	m_head = static_cast< uint8_t* >(m_allocator->alloc(totalSize, 16, L"Pool"));
	m_tail = m_head;

	T_ASSERT (m_head);

	std::memset(m_head, 0, totalSize);
}

PoolAllocator::~PoolAllocator()
{
	T_EXCEPTION_GUARD_BEGIN;

	m_allocator->free(m_head);

	T_EXCEPTION_GUARD_END;
}

void PoolAllocator::enter()
{
	m_scope.push(m_tail);
}

void PoolAllocator::leave()
{
	m_tail = m_scope.top();
	m_scope.pop();
}

void* PoolAllocator::alloc(uint32_t size)
{
	T_ASSERT_M (uint32_t(m_tail - m_head) + size < m_totalSize, L"Out of memory");
	void* ptr = m_tail;
	m_tail += size;
	return ptr;
}

}

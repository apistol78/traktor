#pragma once

#include <vector>
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/Align.h"

namespace traktor
{

/*!
 * \ingroup Core
 */
template < typename BoxedType, int BoxesPerBlock >
class BoxedAllocator
{
public:
	virtual ~BoxedAllocator()
	{
		for (auto allocator : m_allocators)
		{
			Alloc::freeAlign(allocator->top());
			delete allocator;
		}
	}

	void* alloc()
	{
#if defined(T_BOXES_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
#endif
		void* ptr = nullptr;
		for (auto allocator : m_allocators)
		{
			if ((ptr = allocator->alloc()) != nullptr)
				return ptr;
		}

		// No more space in block allocators; create a new block allocator.
		void* top = Alloc::acquireAlign(BoxesPerBlock * sizeof(BoxedType), alignOf< BoxedType >(), T_FILE_LINE);
		T_FATAL_ASSERT_M (top, L"Out of memory");

		BlockAllocator* allocator = new BlockAllocator(top, BoxesPerBlock, sizeof(BoxedType));
		T_FATAL_ASSERT_M (allocator, L"Out of memory");

		m_allocators.push_back(allocator);
		return allocator->alloc();
	}

	void free(void* ptr)
	{
#if defined(T_BOXES_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
#endif
		for (auto allocator : m_allocators)
		{
			if (allocator->free(ptr))
				return;
		}
		T_FATAL_ERROR;
	}

private:
#if defined(T_BOXES_USE_MT_LOCK)
	SpinLock m_lock;
#endif
	std::vector< BlockAllocator* > m_allocators;
};

}

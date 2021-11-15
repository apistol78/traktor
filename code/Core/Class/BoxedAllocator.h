#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/Align.h"

namespace traktor
{

/*! Specialized allocator for boxed values.
 * \ingroup Core
 */
template < typename BoxedType, int BoxesPerBlock >
class BoxedAllocator
{
public:
	virtual ~BoxedAllocator()
	{
		for (auto allocator : m_allocators)
			Alloc::freeAlign(allocator.top());
	}

	void* alloc()
	{
#if defined(T_BOXES_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
#endif
		void* ptr = nullptr;

		// First try last successful allocator.
		if (m_allocator)
		{
			if ((ptr = m_allocator->alloc()) != nullptr)
				return ptr;
		}

		// Try all allocators, remember one which was successful.
		for (auto& allocator : m_allocators)
		{
			if ((ptr = allocator.alloc()) != nullptr)
			{
				m_allocator = &allocator;
				return ptr;
			}
		}

		// No more space in block allocators; create a new block allocator.
		void* top = Alloc::acquireAlign(BoxesPerBlock * sizeof(BoxedType), alignOf< BoxedType >(), T_FILE_LINE);
		T_FATAL_ASSERT_M (top, L"Out of memory");
		
		m_allocators.push_back(BlockAllocator(top, BoxesPerBlock, sizeof(BoxedType)));
		m_allocator = &m_allocators.back();

		return m_allocator->alloc();
	}

	void free(void* ptr)
	{
#if defined(T_BOXES_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
#endif
		for (auto& allocator : m_allocators)
		{
			if (allocator.free(ptr))
				return;
		}
		T_FATAL_ERROR;
	}

private:
#if defined(T_BOXES_USE_MT_LOCK)
	SpinLock m_lock;
#endif
	AlignedVector< BlockAllocator > m_allocators;
	BlockAllocator* m_allocator = nullptr;
};

}

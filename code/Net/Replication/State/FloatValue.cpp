/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Net/Replication/State/FloatValue.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

template < typename ValueType >
class ValueAllocator
{
public:
	enum { ValuesPerBlock = 4096 };

	void* alloc()
	{
		void* ptr = 0;
		for (std::vector< BlockAllocator* >::iterator i = m_allocators.begin(); i != m_allocators.end(); ++i)
		{
			BlockAllocator* allocator = *i;
			if ((ptr = allocator->alloc()) != 0)
				return ptr;
		}

		// No more space in block allocators; create a new block allocator.
		void* top = Alloc::acquireAlign(ValuesPerBlock * sizeof(ValueType), alignOf< ValueType >(), T_FILE_LINE);
		T_FATAL_ASSERT_M (top, L"Out of memory");

		BlockAllocator* allocator = new BlockAllocator(top, ValuesPerBlock, sizeof(ValueType));
		T_FATAL_ASSERT_M (allocator, L"Out of memory");

		m_allocators.push_back(allocator);
		return allocator->alloc();
	}

	void free(void* ptr)
	{
		for (std::vector< BlockAllocator* >::iterator i = m_allocators.begin(); i != m_allocators.end(); ++i)
		{
			BlockAllocator* allocator = *i;
			if (allocator->free(ptr))
				return;
		}

		T_FATAL_ERROR;
	}

private:
	std::vector< BlockAllocator* > m_allocators;
};

ValueAllocator< FloatValue > s_valueAllocator;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.FloatValue", FloatValue, IValue)

void* FloatValue::operator new (size_t size)
{
	T_ASSERT (size == sizeof(FloatValue));
	return s_valueAllocator.alloc();
}

void FloatValue::operator delete (void* ptr)
{
	s_valueAllocator.free(ptr);
}

	}
}

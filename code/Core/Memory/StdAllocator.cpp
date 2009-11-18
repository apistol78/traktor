#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{

void* StdAllocator::alloc(size_t size, size_t align)
{
	void* ptr = Alloc::acquireAlign(size, align);
	T_ASSERT (ptr);
	return ptr;
}

void StdAllocator::free(void* ptr)
{
	Alloc::freeAlign(ptr);
}

IAllocator::MemoryType StdAllocator::type(void* ptr) const
{
	return MtUnknown;
}

}

#include "Core/Heap/StdAllocator.h"
#include "Core/Heap/Alloc.h"

namespace traktor
{

void* StdAllocator::alloc(size_t size, size_t align)
{
	void* ptr = allocAlign(size, align);
	T_ASSERT (ptr);
	return ptr;
}

void StdAllocator::free(void* ptr)
{
	freeAlign(ptr);
}

Allocator::MemoryType StdAllocator::type(void* ptr) const
{
	return MtUnknown;
}

}

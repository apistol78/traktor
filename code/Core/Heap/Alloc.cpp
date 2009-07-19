#include <cstdlib>
#include "Core/Heap/Alloc.h"

namespace traktor
{

void* Alloc::acquire(size_t size)
{
	return std::malloc(size);
}

void Alloc::free(void* ptr)
{
	std::free(ptr);
}

void* Alloc::acquireAlign(size_t size, size_t align)
{
#if defined(_WIN32) && !defined(WINCE)
	return _aligned_malloc(size, align);
#else
	return alloc(size);
#endif
}

void Alloc::freeAlign(void* ptr)
{
#if defined(_WIN32) && !defined(WINCE)
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

}

#include <cstdlib>
#include "Core/Heap/Alloc.h"

namespace traktor
{

void* alloc(size_t size)
{
	return std::malloc(size);
}

void free(void* ptr)
{
	std::free(ptr);
}

void* allocAlign(size_t size, size_t align)
{
#if defined(_WIN32) && !defined(WINCE)
	return _aligned_malloc(size, align);
#else
	return alloc(size);
#endif
}

void freeAlign(void* ptr)
{
#if defined(_WIN32) && !defined(WINCE)
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

}

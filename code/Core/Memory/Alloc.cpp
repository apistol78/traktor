#include <cstdlib>
#include "Core/Memory/Alloc.h"

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
	uint8_t* ptr = static_cast< uint8_t* >(acquire(size + align));
	if (!ptr)
		return 0;

	ptr = reinterpret_cast< uint8_t* >((size_t(ptr) + (align - 1)) & ~(align - 1));
	return ptr;
#endif
}

void Alloc::freeAlign(void* ptr)
{
#if defined(_WIN32) && !defined(WINCE)
	_aligned_free(ptr);
#else
	// \note This pointer might be slight off as we compensate for alignment in acquireAlign.
	free(ptr);
#endif
}

}

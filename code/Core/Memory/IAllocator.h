#pragma once

#include <cstddef>

namespace traktor
{

/*! Allocator interface.
 * \ingroup Core
 *
 * Allocators used by the Heap must implement this interface.
 */
class IAllocator
{
public:
	virtual void* alloc(size_t size, size_t align, const char* const tag) = 0;

	virtual void free(void* ptr) = 0;
};

}


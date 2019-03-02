#pragma once

#include "Core/Memory/IAllocator.h"

namespace traktor
{

/*! \brief Standard allocator.
 * \ingroup Core
 *
 * The standard allocator uses the standard malloc and free
 * from the C runtime in order to allocate memory.
 */
class StdAllocator : public RefCountImpl< IAllocator >
{
public:
	virtual void* alloc(size_t size, size_t align, const char* const tag) override final;

	virtual void free(void* ptr) override final;
};

}


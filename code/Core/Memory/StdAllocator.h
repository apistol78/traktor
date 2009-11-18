#ifndef traktor_StdAllocator_H
#define traktor_StdAllocator_H

#include "Core/Memory/IAllocator.h"

namespace traktor
{

/*! \brief Standard allocator.
 * \ingroup Core
 *
 * The standard allocator uses the standard malloc and free
 * from the C runtime in order to allocate memory. Thus
 * it's not as efficient for small object allocations such
 * as the FastAllocator.
 */
class StdAllocator : public IAllocator
{
public:
	virtual void* alloc(size_t size, size_t align);
	
	virtual void free(void* ptr);

	virtual MemoryType type(void* ptr) const;
};

}

#endif	// traktor_StdAllocator_H

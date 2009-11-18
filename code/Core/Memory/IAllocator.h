#ifndef traktor_IAllocator_H
#define traktor_IAllocator_H

#include <cstddef>
#include "Core/IRefCount.h"

namespace traktor
{

/*! \brief Allocator interface.
 * \ingroup Core
 *
 * Allocators used by the Heap must implement this interface.
 */
class IAllocator : public IRefCount
{
public:
	enum MemoryType
	{
		MtUnknown,
		MtStack,
		MtAllocated,
		MtFreed,
		MtDebug
	};

	virtual void* alloc(size_t size, size_t align) = 0;
	
	virtual void free(void* ptr) = 0;

	virtual MemoryType type(void* ptr) const = 0;
};

}

#endif	// traktor_IAllocator_H

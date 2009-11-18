#ifndef traktor_Allocator_H
#define traktor_Allocator_H

#include <cstddef>

namespace traktor
{

/*! \brief Allocator interface.
 * \ingroup Core
 *
 * Allocators used by the Heap must implement this interface.
 */
class Allocator
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

	virtual ~Allocator() {}

	virtual void* alloc(size_t size, size_t align) = 0;
	
	virtual void free(void* ptr) = 0;

	virtual MemoryType type(void* ptr) const = 0;
};

}

#endif	// traktor_Allocator_H

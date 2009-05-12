#ifndef traktor_FastAllocator_H
#define traktor_FastAllocator_H

#include "Core/Heap/Allocator.h"
#include "Core/Thread/CriticalSection.h"

namespace traktor
{
	
class BlockAllocator;

/*! \brief Fast allocator.
 * \ingroup Core
 *
 * The fast allocator is optimized for allocated
 * fixed size chunks for small objects. It uses
 * a greedy O(1) allocation scheme for such allocations.
 * The fast allocator is enabled in release build by default
 * but can be turned off in HeapConfig.h
 */
class FastAllocator : public Allocator
{
public:
	FastAllocator(Allocator* systemAllocator);
	
	virtual ~FastAllocator();
	
	virtual void* alloc(size_t size, size_t align);
	
	virtual void free(void* ptr);

	virtual MemoryType type(void* ptr) const;
	
private:
	CriticalSection m_lock;
	Allocator* m_systemAllocator;
	BlockAllocator* m_blockAlloc[16];
};

}

#endif	// traktor_FastAllocator_H

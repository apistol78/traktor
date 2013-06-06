#ifndef traktor_FastAllocator_H
#define traktor_FastAllocator_H

#include "Core/Ref.h"
#include "Core/Memory/IAllocator.h"

namespace traktor
{
	
class BlockAllocator;

/*! \brief Fast allocator.
 * \ingroup Core
 *
 * The fast allocator is optimized for allocated
 * fixed size chunks for small objects. It uses
 * a greedy O(1) allocation scheme for such allocations.
 */
class FastAllocator : public RefCountImpl< IAllocator >
{
public:
	FastAllocator(IAllocator* systemAllocator);
	
	virtual ~FastAllocator();
	
	virtual void* alloc(size_t size, size_t align, const char* const tag);
	
	virtual void free(void* ptr);
	
private:
	Ref< IAllocator > m_systemAllocator;
	BlockAllocator* m_blockAlloc[5];
	int32_t m_blockAllocLock[5];
};

}

#endif	// traktor_FastAllocator_H

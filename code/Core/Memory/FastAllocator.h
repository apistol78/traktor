#ifndef traktor_FastAllocator_H
#define traktor_FastAllocator_H

#include "Core/Ref.h"
#include "Core/Memory/IAllocator.h"
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
 */
class FastAllocator : public RefCountImpl< IAllocator >
{
public:
	FastAllocator(IAllocator* systemAllocator);
	
	virtual ~FastAllocator();
	
	virtual void* alloc(size_t size, size_t align, const wchar_t* const tag);
	
	virtual void free(void* ptr);
	
private:
	CriticalSection m_lock;
	Ref< IAllocator > m_systemAllocator;
	BlockAllocator* m_blockAlloc[16];
};

}

#endif	// traktor_FastAllocator_H

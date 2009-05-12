#ifndef traktor_DebugAllocator_H
#define traktor_DebugAllocator_H

#include <list>
#include "Core/Heap/Allocator.h"
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

/*! \brief Debug allocator.
 * \ingroup Core
 *
 * Debug allocator uses a couple of mechanisms to ensure
 * no invalid access is made to allocated data.
 * Note that the debug allocator doesn't actually release
 * any memory which it once has allocated in order to ensure
 * data isn't modified after it has been released.
 */
class DebugAllocator : public Allocator
{
public:
	DebugAllocator(Allocator* systemAllocator);

	virtual ~DebugAllocator();

	virtual void* alloc(size_t size, size_t align);
	
	virtual void free(void* ptr);

	virtual MemoryType type(void* ptr) const;

private:
	struct Block
	{
		void* top;
		size_t size;
	};

	mutable CriticalSection m_lock;
	Allocator* m_systemAllocator;
	std::list< Block > m_aliveBlocks;
	std::list< Block > m_freedBlocks;

	void checkBlocks();
};

}

#endif	// traktor_DebugAllocator_H

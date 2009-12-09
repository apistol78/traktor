#ifndef traktor_TrackAllocator_H
#define traktor_TrackAllocator_H

#include <list>
#include "Core/Ref.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

/*! \brief Track allocator.
 * \ingroup Core
 *
 * Track allocator is a light-weight debugging
 * allocator. It will keep a "live"-list to track
 * memory leaks.
 */
class TrackAllocator : public RefCountImpl< IAllocator >
{
public:
	TrackAllocator(IAllocator* systemAllocator);

	virtual ~TrackAllocator();

	virtual void* alloc(size_t size, size_t align, const wchar_t* const tag);

	virtual void free(void* ptr);

private:
	struct Block
	{
		const wchar_t* tag;
		void* top;
		size_t size;
		void* at[4];
	};

	mutable Semaphore m_lock;
	Ref< IAllocator > m_systemAllocator;
	std::list< Block > m_aliveBlocks;
};

}

#endif	// traktor_TrackAllocator_H

#ifndef traktor_TrackAllocator_H
#define traktor_TrackAllocator_H

#include <map>
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

	virtual void* alloc(size_t size, size_t align, const char* const tag);

	virtual void free(void* ptr);

private:
	struct Block
	{
		const char* tag;
		size_t size;
		void* at[8];

		bool operator < (const Block& rh) const;
	};

	struct Stats
	{
		const char* tag;
		uint32_t count;
		size_t memory;

		Stats()
		:	tag(0)
		,	count(0)
		,	memory(0)
		{
		}
	};

	mutable Semaphore m_lock;
	Ref< IAllocator > m_systemAllocator;
	std::map< void*, Block > m_aliveBlocks;
	std::map< void*, Stats > m_allocStats;
};

}

#endif	// traktor_TrackAllocator_H

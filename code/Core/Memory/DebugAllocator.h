/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_DebugAllocator_H
#define traktor_DebugAllocator_H

#include <list>
#include "Core/Ref.h"
#include "Core/Memory/IAllocator.h"
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
class DebugAllocator : public RefCountImpl< IAllocator >
{
public:
	DebugAllocator(IAllocator* systemAllocator);

	virtual ~DebugAllocator();

	virtual void* alloc(size_t size, size_t align, const char* const tag) T_OVERRIDE T_FINAL;
	
	virtual void free(void* ptr) T_OVERRIDE T_FINAL;

private:
	struct Block
	{
		void* top;
		size_t size;
	};

	mutable CriticalSection m_lock;
	Ref< IAllocator > m_systemAllocator;
	std::list< Block > m_aliveBlocks;
	std::list< Block > m_freedBlocks;

	void checkBlocks();
};

}

#endif	// traktor_DebugAllocator_H

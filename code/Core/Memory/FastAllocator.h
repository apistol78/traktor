/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	
	virtual void* alloc(size_t size, size_t align, const char* const tag) T_OVERRIDE T_FINAL;
	
	virtual void free(void* ptr) T_OVERRIDE T_FINAL;
	
private:
	Ref< IAllocator > m_systemAllocator;
	BlockAllocator* m_blockAlloc[5];
	int32_t m_blockAllocLock[5];
	int8_t m_blockAllocFull[5];
};

}

#endif	// traktor_FastAllocator_H

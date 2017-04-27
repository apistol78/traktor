/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual void* alloc(size_t size, size_t align, const char* const tag) = 0;
	
	virtual void free(void* ptr) = 0;
};

}

#endif	// traktor_IAllocator_H

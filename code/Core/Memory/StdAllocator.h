/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_StdAllocator_H
#define traktor_StdAllocator_H

#include "Core/Memory/IAllocator.h"

namespace traktor
{

/*! \brief Standard allocator.
 * \ingroup Core
 *
 * The standard allocator uses the standard malloc and free
 * from the C runtime in order to allocate memory.
 */
class StdAllocator : public RefCountImpl< IAllocator >
{
public:
	virtual void* alloc(size_t size, size_t align, const char* const tag) T_OVERRIDE T_FINAL;
	
	virtual void free(void* ptr) T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_StdAllocator_H

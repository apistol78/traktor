/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{

void* StdAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	void* ptr = Alloc::acquireAlign(size, align, tag);
	T_ASSERT (ptr);
	return ptr;
}

void StdAllocator::free(void* ptr)
{
	Alloc::freeAlign(ptr);
}

}

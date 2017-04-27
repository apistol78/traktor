/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Atomic.h"

namespace traktor
{
	namespace
	{

#pragma pack(1)
struct Block
{
#if defined(_DEBUG)
	uint32_t magic;
#endif
	size_t size;
};
#pragma pack()

#if defined(_DEBUG)
const uint32_t c_magic = 'LIVE';
#endif
int32_t s_count = 0;
int32_t s_allocated = 0;
int32_t s_peek = 0;

	}

void* Alloc::acquire(size_t size, const char* tag)
{
	void* ptr = std::malloc(size + sizeof(Block));
	if (!ptr)
	{
		std::cerr << "Out of memory; trying to allocate " << size << " byte(s)" << std::endl;
		T_FATAL_ERROR;
	}

	Block* block = static_cast< Block* >(ptr);
#if defined(_DEBUG)
	block->magic = c_magic;
#endif
	block->size = size;

	Atomic::increment(s_count);
	
	// If allocated exceed peek than replace peek value.
	int32_t a = Atomic::add(s_allocated, int32_t(size + sizeof(Block)));
	int32_t p;
	Atomic::exchange(p, s_peek);
	if (a > p)
		Atomic::exchange(s_peek, a);

	return block + 1;
}

void Alloc::free(void* ptr)
{
	if (ptr)
	{
		Block* block = static_cast< Block* >(ptr) - 1;
		T_ASSERT_M(block->magic == c_magic, L"Invalid free");
		Atomic::add(s_allocated, -int32_t(block->size + sizeof(Block)));
		std::free(block);
	}
}

void* Alloc::acquireAlign(size_t size, size_t align, const char* tag)
{
	T_ASSERT (align >= 1);

	uint8_t* ptr = (uint8_t*)Alloc::acquire(size + sizeof(intptr_t) + align - 1, tag);
	if (!ptr)
	{
		std::cerr << "Out of memory; trying to allocate " << size << " byte(s)" << std::endl;
		T_FATAL_ERROR;
	}

	uint8_t* alignedPtr = alignUp(ptr + sizeof(intptr_t), align);
	*(intptr_t*)(alignedPtr - sizeof(intptr_t)) = intptr_t(ptr);

#if defined(_DEBUG)
	intptr_t originalPtr = *((intptr_t*)(alignedPtr) - 1);
	Block* block = static_cast< Block* >((void*)originalPtr) - 1;
	T_ASSERT_M(block->magic == c_magic, L"Invalid free");
#endif

	return alignedPtr;
}

void Alloc::freeAlign(void* ptr)
{
	if (ptr)
	{
		intptr_t originalPtr = *((intptr_t*)(ptr) - 1);
		Alloc::free((void*)originalPtr);
	}
}

size_t Alloc::count()
{
	return size_t(s_count);
}

size_t Alloc::allocated()
{
	int32_t p, a;

	// Read peek and allocated values.
	Atomic::exchange(p, s_peek);
	Atomic::exchange(a, s_allocated);

	// Reset peek value.
	Atomic::exchange(s_peek, 0);

	return size_t(p > a ? p : a);
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#if defined(_WIN32)
#	define T_USE_MIMALLOC
#endif

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"

#if defined(T_USE_MIMALLOC)
#	include "mimalloc.h"
#endif

namespace traktor
{
	namespace
	{

#pragma pack(1)
struct Block
{
	uint32_t magic;
	size_t size;
};
#pragma pack()

const uint32_t c_magic = 'LIVE';
std::atomic< int32_t > s_count(0);
std::atomic< int64_t > s_allocated(0);

	}

void* Alloc::acquire(size_t size, const char* tag)
{
#if defined(T_USE_MIMALLOC)
	return mi_malloc(size + sizeof(Block));
#else
	void* ptr = std::malloc(size + sizeof(Block));
	if (!ptr)
		T_FATAL_ERROR;

	Block* block = static_cast< Block* >(ptr);
	block->magic = c_magic;
	block->size = size;

	s_count++;
	s_allocated += size + sizeof(Block);
	return block + 1;
#endif
}

void Alloc::free(void* ptr)
{
#if defined(T_USE_MIMALLOC)
		mi_free(ptr);
#else
	if (ptr)
	{
		Block* block = (Block*)ptr - 1;
		T_FATAL_ASSERT_M(block->magic == c_magic, L"Invalid free");
		s_allocated -= (int64_t)(block->size + sizeof(Block));
		std::free(block);
	}
#endif
}

void* Alloc::acquireAlign(size_t size, size_t align, const char* tag)
{
	T_ASSERT(align >= 1);
#if defined(T_USE_MIMALLOC)
	return mi_malloc_aligned(size, align);
#else
	uint8_t* ptr = (uint8_t*)Alloc::acquire(size + sizeof(intptr_t) + align - 1, tag);
	if (!ptr)
		T_FATAL_ERROR;

	uint8_t* alignedPtr = alignUp(ptr + sizeof(intptr_t), align);
	*(intptr_t*)(alignedPtr - sizeof(intptr_t)) = (intptr_t)ptr;

	return alignedPtr;
#endif
}

void Alloc::freeAlign(void* ptr)
{
#if defined(T_USE_MIMALLOC)
	mi_free(ptr);
#else
	if (ptr)
	{
		intptr_t originalPtr = *((intptr_t*)(ptr) - 1);
		Alloc::free((void*)originalPtr);
	}
#endif
}

size_t Alloc::count()
{
	return (size_t)s_count;
}

size_t Alloc::allocated()
{
	return (size_t)s_allocated;
}

}

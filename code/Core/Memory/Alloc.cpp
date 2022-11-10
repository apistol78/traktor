/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"

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
	void* ptr = std::malloc(size + sizeof(Block));
	if (!ptr)
		T_FATAL_ERROR;

	Block* block = static_cast< Block* >(ptr);
	block->magic = c_magic;
	block->size = size;

	s_count++;
	s_allocated += size + sizeof(Block);
	return block + 1;
}

void Alloc::free(void* ptr)
{
	if (ptr)
	{
		Block* block = (Block*)ptr - 1;
		T_FATAL_ASSERT_M(block->magic == c_magic, L"Invalid free");
		s_allocated -= (int64_t)(block->size + sizeof(Block));
		std::free(block);
	}
}

void* Alloc::acquireAlign(size_t size, size_t align, const char* tag)
{
	T_ASSERT(align >= 1);

	uint8_t* ptr = (uint8_t*)Alloc::acquire(size + sizeof(intptr_t) + align - 1, tag);
	if (!ptr)
		T_FATAL_ERROR;

	uint8_t* alignedPtr = alignUp(ptr + sizeof(intptr_t), align);
	*(intptr_t*)(alignedPtr - sizeof(intptr_t)) = (intptr_t)ptr;

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
	return (size_t)s_count;
}

size_t Alloc::allocated()
{
	return (size_t)s_allocated;
}

}

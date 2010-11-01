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
	size_t size;
};
#pragma pack()

int32_t s_allocated = 0;

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
	block->size = size;

	Atomic::add(s_allocated, int32_t(size));
	return block + 1;
}

void Alloc::free(void* ptr)
{
	if (ptr)
	{
		Block* block = static_cast< Block* >(ptr) - 1;
		Atomic::add(s_allocated, -int32_t(block->size));
		std::free(block);
	}
}

void* Alloc::acquireAlign(size_t size, size_t align, const char* tag)
{
	// Ensure minimum alignment.
	if (align < 4)
		align = 4;

	uint8_t* ptr = (uint8_t*)acquire(size + sizeof(intptr_t) + align, tag);
	if (!ptr)
	{
		std::cerr << "Out of memory; trying to allocate " << size << " byte(s)" << std::endl;
		T_FATAL_ERROR;
	}
	
	uint8_t* alignedPtr = alignUp(ptr + sizeof(intptr_t), align);
	*(intptr_t*)(alignedPtr - sizeof(intptr_t)) = intptr_t(ptr);
	return alignedPtr;

}

void Alloc::freeAlign(void* ptr)
{
	if (ptr)
	{
		intptr_t originalPtr = *((intptr_t*)(ptr) - 1);
		free((void*)originalPtr);
	}
}

size_t Alloc::allocated()
{
	return s_allocated;
}

}

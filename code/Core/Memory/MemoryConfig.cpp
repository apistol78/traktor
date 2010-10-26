#include "Core/Memory/Alloc.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Memory/SystemConstruct.h"
#include "Core/Memory/TrackAllocator.h"

namespace traktor
{
	namespace
	{

IAllocator* s_allocator = 0;

void destroyAllocator()
{
	freeDestruct(s_allocator);
	s_allocator = 0;
}

	}

IAllocator* getAllocator()
{
	if (!s_allocator)
	{
		StdAllocator* stdAllocator = allocConstruct< StdAllocator >();
#if !defined(_DEBUG)
		s_allocator = allocConstruct< FastAllocator >(stdAllocator);
#else
		s_allocator = allocConstruct< TrackAllocator >(stdAllocator);
#endif
		std::atexit(destroyAllocator);
	}
	return s_allocator;
}

}

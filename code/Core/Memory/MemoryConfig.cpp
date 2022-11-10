/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstdlib>
#include "Core/Memory/Alloc.h"
#include "Core/Memory/DebugAllocator.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Memory/SystemConstruct.h"
#include "Core/Memory/TrackAllocator.h"

namespace traktor
{
	namespace
	{

IAllocator* s_stdAllocator = nullptr;
IAllocator* s_allocator = nullptr;

#if !defined(__MAC__) && !defined(__IOS__)
void destroyAllocator()
{
	if (s_allocator != s_stdAllocator)
		freeDestruct(s_allocator);

	freeDestruct(s_stdAllocator);

	s_stdAllocator = nullptr;
	s_allocator = nullptr;
}
#endif

	}

IAllocator* getAllocator()
{
	if (!s_allocator)
	{
		s_stdAllocator = allocConstruct< StdAllocator >();

#if defined(__ANDROID__) || defined(__APPLE__)
		s_allocator = s_stdAllocator;
#elif !defined(_DEBUG) || defined(__LINUX__) || defined(__RPI__)
		//s_allocator = allocConstruct< FastAllocator >(s_stdAllocator);
		s_allocator = s_stdAllocator;
#else
		s_allocator = allocConstruct< TrackAllocator >(s_stdAllocator);
		//s_allocator = allocConstruct< DebugAllocator >(s_stdAllocator);
		//s_allocator = s_stdAllocator;
#endif

#if !defined(__MAC__) && !defined(__IOS__)
		std::atexit(destroyAllocator);
#endif
	}
	return s_allocator;
}

}

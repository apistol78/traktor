/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{

void* StdAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	void* ptr = Alloc::acquireAlign(size, align, tag);
	T_ASSERT(ptr);
	return ptr;
}

void StdAllocator::free(void* ptr)
{
	Alloc::freeAlign(ptr);
}

}

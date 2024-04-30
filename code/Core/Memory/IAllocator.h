/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstddef>

namespace traktor
{

/*! Allocator interface.
 * \ingroup Core
 *
 * Allocators used by the Heap must implement this interface.
 */
class IAllocator
{
public:
	virtual ~IAllocator() {}

	[[nodiscard]] virtual void* alloc(size_t size, size_t align, const char* const tag) = 0;

	virtual void free(void* ptr) = 0;
};

}


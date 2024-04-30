/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Memory/IAllocator.h"

namespace traktor
{

/*! Standard allocator.
 * \ingroup Core
 *
 * The standard allocator uses the standard malloc and free
 * from the C runtime in order to allocate memory.
 */
class StdAllocator : public IAllocator
{
public:
	[[nodiscard]] virtual void* alloc(size_t size, size_t align, const char* const tag) override final;

	virtual void free(void* ptr) override final;
};

}


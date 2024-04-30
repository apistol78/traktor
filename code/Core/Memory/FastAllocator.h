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

class BlockAllocator;

/*! Fast allocator.
 * \ingroup Core
 *
 * The fast allocator is optimized for allocated
 * fixed size chunks for small objects. It uses
 * a greedy O(1) allocation scheme for such allocations.
 */
class FastAllocator : public IAllocator
{
public:
	explicit FastAllocator(IAllocator* systemAllocator);

	virtual ~FastAllocator();

	[[nodiscard]] virtual void* alloc(size_t size, size_t align, const char* const tag) override final;

	virtual void free(void* ptr) override final;

private:
	IAllocator* m_systemAllocator;
	BlockAllocator* m_blockAlloc[6];
	int32_t m_blockAllocLock[6];
	int8_t m_blockAllocFull[6];
};

}


/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Memory/IAllocator.h"

namespace traktor
{

/*! Dynamic fast allocator.
 * \ingroup Core
 *
 * Like FastAllocator, this allocator is optimized for small, fixed-size
 * allocations using a greedy O(1) block allocation scheme. Unlike
 * FastAllocator, whose per-size pools have a fixed capacity and silently
 * fall back to the system allocator once exhausted, this allocator backs
 * each quantized size with a growable list of block allocators (chunks).
 * When every chunk of a size is full a new chunk is created on demand, so
 * the fast path keeps serving allocations as long as system memory lasts.
 */
class DynamicFastAllocator : public IAllocator
{
public:
	explicit DynamicFastAllocator(IAllocator* systemAllocator);

	virtual ~DynamicFastAllocator();

	[[nodiscard]] virtual void* alloc(size_t size, size_t align, const char* const tag) override final;

	virtual void free(void* ptr) override final;

private:
	struct Chunk;

	constexpr static int32_t QuantizeCount = 6;

	struct SizeClass
	{
		Chunk* chunks;		 //!< All chunks for this size, newest first.
		Chunk* active;		 //!< Cached chunk most likely to have a free block.
		uintptr_t envMin;	 //!< Lowest block address across all chunks (fast free rejection).
		uintptr_t envMax;	 //!< End of highest block address across all chunks.
		uint32_t qsize;		 //!< Quantized block size in bytes.
		uint32_t blockCount; //!< Number of blocks per chunk.
		int32_t lock;		 //!< Spin-lock guarding this size class.
	};

	IAllocator* m_systemAllocator;
	SizeClass m_sizeClass[QuantizeCount];

	Chunk* grow(SizeClass& sc, const char* const tag);
};

}

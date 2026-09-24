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

#include <atomic>

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
 *
 * Chunks are aligned to, and sized in, whole granules. A two-level radix
 * map from granule to owning chunk makes free() O(1) regardless of how
 * many chunks exist; pointers not found in the map belong to the system
 * allocator.
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
	constexpr static int32_t GranuleBits = 16;	//!< 64 KiB granules.
	constexpr static int32_t LeafBits = 16;		//!< Granules per leaf; a leaf spans 4 GiB.
	constexpr static int32_t RootBits = 48 - GranuleBits - LeafBits;

	struct SizeClass
	{
		Chunk* chunks;		 //!< All chunks for this size, newest first.
		Chunk* active;		 //!< Cached chunk most likely to have a free block.
		uint32_t qsize;		 //!< Quantized block size in bytes.
		uint32_t blockCount; //!< Number of blocks per chunk.
		int32_t lock;		 //!< Spin-lock guarding this size class.
	};

	IAllocator* m_systemAllocator;
	SizeClass m_sizeClass[QuantizeCount];
	std::atomic< std::atomic< Chunk* >* > m_map[1 << RootBits]; //!< Granule to chunk; leaves created on demand, never released until destruction.

	Chunk* grow(SizeClass& sc, const char* const tag);

	bool mapChunk(uintptr_t base, size_t size, Chunk* chunk);

	Chunk* findChunk(uintptr_t p) const;
};

}

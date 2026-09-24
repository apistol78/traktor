/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Memory/DynamicFastAllocator.h"

#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

#include <new>

namespace traktor
{
namespace
{

// Number of blocks in each chunk, per quantized size. A grown chunk uses the
// same count, so the first chunk matches FastAllocator's fixed capacity.
const uint32_t c_blockCounts[] = {
#if defined(__IOS__) || defined(__ANDROID__)
	32768, // 16
	36864, // 32
	20480, // 64
	8192,  // 128
	4096,  // 256
	128	   // 512
#else
	131072, // 16
	131072, // 32
	40960,	// 64
	16384,	// 128
	16384,	// 256
	1024	// 512
#endif
};

T_FORCE_INLINE void spinLock(int32_t& lock)
{
	while (Atomic::exchange(lock, 1) != 0)
		ThreadManager::getInstance().getCurrentThread()->yield();
}

T_FORCE_INLINE void spinUnlock(int32_t& lock)
{
	Atomic::exchange(lock, 0);
}

}

struct DynamicFastAllocator::Chunk
{
	Chunk* next;
	SizeClass* sizeClass;
	BlockAllocator allocator;

	explicit Chunk(SizeClass* sizeClass, void* blocks, int32_t count, size_t size)
		: next(nullptr)
		, sizeClass(sizeClass)
		, allocator(blocks, count, size)
	{
	}
};

DynamicFastAllocator::DynamicFastAllocator(IAllocator* systemAllocator)
	: m_systemAllocator(systemAllocator)
{
	for (auto& leaf : m_map)
		leaf.store(nullptr, std::memory_order_relaxed);

	for (uint32_t i = 0; i < QuantizeCount; ++i)
	{
		SizeClass& sc = m_sizeClass[i];
		sc.chunks = nullptr;
		sc.active = nullptr;
		sc.qsize = 1U << (i + 4);
		sc.blockCount = c_blockCounts[i];
		sc.lock = 0;
		grow(sc, T_FILE_LINE);
	}
}

DynamicFastAllocator::~DynamicFastAllocator()
{
	for (uint32_t i = 0; i < QuantizeCount; ++i)
	{
		Chunk* c = m_sizeClass[i].chunks;
		while (c)
		{
			Chunk* next = c->next;
			c->~Chunk();
			m_systemAllocator->free(c);
			c = next;
		}
	}

	for (auto& leaf : m_map)
	{
		std::atomic< Chunk* >* granules = leaf.load(std::memory_order_relaxed);
		if (granules)
			m_systemAllocator->free(granules);
	}
}

void* DynamicFastAllocator::alloc(size_t size, size_t align, const char* const tag)
{
	if (size > 0 && size <= 512 && align <= 16)
	{
		if (size < 16)
			size = 16;

		size = nearestLog2((uint32_t)size);

		const uint32_t qid = log2((uint32_t)size) - 4;
		SizeClass& sc = m_sizeClass[qid];
		void* p = nullptr;

		spinLock(sc.lock);

		// Fast path: the cached active chunk usually has a free block.
		if (sc.active)
			p = sc.active->allocator.alloc();

		// Active chunk exhausted; reuse a block reclaimed in another chunk.
		if (!p)
		{
			for (Chunk* c = sc.chunks; c; c = c->next)
			{
				if ((p = c->allocator.alloc()) != nullptr)
				{
					sc.active = c;
					break;
				}
			}
		}

		// All chunks are full; grow by allocating another chunk.
		if (!p)
		{
			Chunk* c = grow(sc, tag);
			if (c)
				p = c->allocator.alloc();
		}

		spinUnlock(sc.lock);

		if (p)
		{
			T_ASSERT(alignUp((uint8_t*)p, 16) == p);
			return p;
		}
	}

	// Fallback for large or over-aligned requests (and the rare growth failure).
	void* p = m_systemAllocator->alloc(size, align, tag);
	T_ASSERT(alignUp((uint8_t*)p, align) == p);
	return p;
}

void DynamicFastAllocator::free(void* ptr)
{
	Chunk* c = findChunk((uintptr_t)ptr);
	if (c)
	{
		SizeClass& sc = *c->sizeClass;
		spinLock(sc.lock);
		c->allocator.free(ptr);
		sc.active = c;
		spinUnlock(sc.lock);
		return;
	}

	m_systemAllocator->free(ptr);
}

DynamicFastAllocator::Chunk* DynamicFastAllocator::grow(SizeClass& sc, const char* const tag)
{
	const size_t granuleSize = (size_t)1 << GranuleBits;
	const size_t headerSize = alignUp(sizeof(Chunk), (size_t)16);

	// Chunk occupies whole granules exclusively so each granule maps to exactly one owner.
	const size_t total = alignUp(headerSize + (size_t)sc.qsize * sc.blockCount, granuleSize);

	void* mem = m_systemAllocator->alloc(total, granuleSize, tag);
	if (!mem)
		return nullptr;

	void* blocks = (uint8_t*)mem + headerSize;
	Chunk* c = new (mem) Chunk(&sc, blocks, (int32_t)sc.blockCount, sc.qsize);

	// Map must be populated before any block is handed out; freeing a block
	// requires finding its chunk.
	if (!mapChunk((uintptr_t)mem, total, c))
	{
		c->~Chunk();
		m_systemAllocator->free(mem);
		return nullptr;
	}

	c->next = sc.chunks;
	sc.active = c;
	Atomic::exchange(sc.chunks, c);
	return c;
}

bool DynamicFastAllocator::mapChunk(uintptr_t base, size_t size, Chunk* chunk)
{
	const uintptr_t first = base >> GranuleBits;
	const uintptr_t last = (base + size - 1) >> GranuleBits;

	// Outside of the addressable range of the map, shouldn't happen with 48-bit user space.
	if ((last >> (LeafBits + RootBits)) != 0)
		return false;

	// Ensure all leaves exist before writing any granule, a failure must leave no stale entries.
	for (uintptr_t leaf = first >> LeafBits; leaf <= (last >> LeafBits); ++leaf)
	{
		std::atomic< std::atomic< Chunk* >* >& root = m_map[leaf];
		if (root.load(std::memory_order_acquire) != nullptr)
			continue;

		const size_t leafSize = sizeof(std::atomic< Chunk* >) << LeafBits;
		void* mem = m_systemAllocator->alloc(leafSize, 16, T_FILE_LINE);
		if (!mem)
			return false;

		std::atomic< Chunk* >* created = (std::atomic< Chunk* >*)mem;
		for (uint32_t i = 0; i < (1U << LeafBits); ++i)
			new (&created[i]) std::atomic< Chunk* >(nullptr);

		// Another thread might have grown into the same leaf concurrently.
		std::atomic< Chunk* >* expected = nullptr;
		if (!root.compare_exchange_strong(expected, created, std::memory_order_acq_rel))
			m_systemAllocator->free(mem);
	}

	for (uintptr_t g = first; g <= last; ++g)
	{
		std::atomic< Chunk* >* granules = m_map[g >> LeafBits].load(std::memory_order_acquire);
		granules[g & ((1U << LeafBits) - 1)].store(chunk, std::memory_order_release);
	}

	return true;
}

DynamicFastAllocator::Chunk* DynamicFastAllocator::findChunk(uintptr_t p) const
{
	const uintptr_t g = p >> GranuleBits;
	if ((g >> (LeafBits + RootBits)) != 0)
		return nullptr;

	const std::atomic< Chunk* >* granules = m_map[g >> LeafBits].load(std::memory_order_acquire);
	if (!granules)
		return nullptr;

	return granules[g & ((1U << LeafBits) - 1)].load(std::memory_order_acquire);
}

}

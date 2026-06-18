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
	uintptr_t base;
	uintptr_t end;
	BlockAllocator allocator;

	explicit Chunk(void* blocks, int32_t count, size_t size)
		: next(nullptr)
		, base((uintptr_t)blocks)
		, end((uintptr_t)blocks + (uintptr_t)count * size)
		, allocator(blocks, count, size)
	{
	}

	bool belong(uintptr_t p) const { return p >= base && p < end; }
};

DynamicFastAllocator::DynamicFastAllocator(IAllocator* systemAllocator)
	: m_systemAllocator(systemAllocator)
{
	for (uint32_t i = 0; i < QuantizeCount; ++i)
	{
		SizeClass& sc = m_sizeClass[i];
		sc.chunks = nullptr;
		sc.active = nullptr;
		sc.envMin = ~(uintptr_t)0;
		sc.envMax = 0;
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
}

DynamicFastAllocator::Chunk* DynamicFastAllocator::grow(SizeClass& sc, const char* const tag)
{
	const size_t headerSize = alignUp(sizeof(Chunk), (size_t)16);
	const size_t total = headerSize + (size_t)sc.qsize * sc.blockCount;

	void* mem = m_systemAllocator->alloc(total, 16, tag);
	if (!mem)
		return nullptr;

	void* blocks = (uint8_t*)mem + headerSize;
	Chunk* c = new (mem) Chunk(blocks, (int32_t)sc.blockCount, sc.qsize);

	sc.envMin = std::min(sc.envMin, c->base);
	sc.envMax = std::max(sc.envMax, c->end);

	c->next = sc.chunks;
	sc.active = c;
	Atomic::exchange(sc.chunks, c);
	return c;
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
	const uintptr_t p = (uintptr_t)ptr;

	for (uint32_t i = 0; i < QuantizeCount; ++i)
	{
		SizeClass& sc = m_sizeClass[i];

		if (p < sc.envMin || p >= sc.envMax)
			continue;

		for (Chunk* c = sc.chunks; c; c = c->next)
		{
			if (c->belong(p))
			{
				spinLock(sc.lock);
				c->allocator.free(ptr);
				sc.active = c;
				spinUnlock(sc.lock);
				return;
			}
		}
	}

	m_systemAllocator->free(ptr);
}

}

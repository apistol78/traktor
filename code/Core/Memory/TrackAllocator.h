/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Memory/IAllocator.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

/*! Track allocator.
 * \ingroup Core
 *
 * Track allocator is a light-weight debugging
 * allocator. It will keep a "live"-list to track
 * memory leaks.
 */
class TrackAllocator : public IAllocator
{
public:
	explicit TrackAllocator(IAllocator* systemAllocator);

	virtual ~TrackAllocator();

	[[nodiscard]] virtual void* alloc(size_t size, size_t align, const char* const tag) override final;

	virtual void free(void* ptr) override final;

private:
	struct Block
	{
		const char* tag;
		size_t size;
		void* at[8];

		bool operator < (const Block& rh) const;
	};

	struct Stats
	{
		const char* tag = nullptr;
		uint32_t count = 0;
		size_t memory = 0;
	};

	mutable Semaphore m_lock;
	IAllocator* m_systemAllocator;
	std::map< void*, Block > m_aliveBlocks;
	std::map< void*, Stats > m_allocStats;
};

}


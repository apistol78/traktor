/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Memory/IAllocator.h"
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

/*! Debug allocator.
 * \ingroup Core
 *
 * Debug allocator uses a couple of mechanisms to ensure
 * no invalid access is made to allocated data.
 * Note that the debug allocator doesn't actually release
 * any memory which it once has allocated in order to ensure
 * data isn't modified after it has been released.
 */
class DebugAllocator : public IAllocator
{
public:
	explicit DebugAllocator(IAllocator* systemAllocator);

	virtual ~DebugAllocator();

	[[nodiscard]] virtual void* alloc(size_t size, size_t align, const char* const tag) override final;

	virtual void free(void* ptr) override final;

private:
	struct Block
	{
		uint8_t* top;
		size_t size;
		void* at[8];
	};

	mutable CriticalSection m_lock;
	IAllocator* m_systemAllocator;
	std::list< Block > m_aliveBlocks;
	std::list< Block > m_freedBlocks;
	int32_t m_untilCheck;

	void checkBlocks();
};

}


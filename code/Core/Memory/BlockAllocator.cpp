/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Memory/BlockAllocator.h"

namespace traktor
{

BlockAllocator::BlockAllocator(void* top, int32_t count, size_t size)
:	m_top(nullptr)
,	m_end(nullptr)
,	m_free(nullptr)
#if defined(_DEBUG)
,	m_alloced(0)
,	m_size(size)
,	m_full(false)
#endif
{
	T_ASSERT((size % sizeof(intptr_t)) == 0);
	const size_t blockSize = size / sizeof(intptr_t);

	m_top = (intptr_t*)top;
	m_end = m_top + count * blockSize;

	int32_t i; size_t j;
	for (i = 0, j = 0; i < count - 1; ++i, j += blockSize)
		m_top[j] = (intptr_t)(&m_top[j + blockSize]);

	m_top[j] = 0;
	m_free = m_top;
}

void* BlockAllocator::alloc()
{
	void* p = nullptr;

	if (m_free)
	{
		p = m_free;
		m_free = (intptr_t*)(*m_free);
		T_ASSERT(m_free >= m_top && m_free < m_end || m_free == nullptr);

#if defined(_DEBUG)
		m_alloced++;
#endif
	}
#if defined(_DEBUG)
	else if (!m_full)
	{
		// Break here to see which block size.
		m_full = true;
	}
#endif

	T_ASSERT(((intptr_t*)p >= m_top && (intptr_t*)p < m_end) || p == nullptr);
	return p;
}

bool BlockAllocator::free(void* p)
{
	intptr_t* block = (intptr_t*)p;
	if (block < m_top || block >= m_end)
		return false;

	*block = (intptr_t)m_free;
	m_free = block;
	T_ASSERT(m_free >= m_top && m_free < m_end);

#if defined (_DEBUG)
	m_alloced--;
#endif
	return true;
}

}

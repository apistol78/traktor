/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Memory/BlockAllocator.h"

namespace traktor
{

BlockAllocator::BlockAllocator(void* top, int count, size_t size)
:	m_top(0)
,	m_end(0)
,	m_free(0)
#if defined(_DEBUG)
,	m_alloced(0)
,	m_size(size)
,	m_full(false)
#endif
{
	T_ASSERT ((size % sizeof(size_t)) == 0);
	size_t blockSize = size / sizeof(size_t);

	m_top = static_cast< size_t* >(top);
	m_end = m_top + count * blockSize;
	
	int i; size_t j;
	for (i = 0, j = 0; i < count - 1; ++i, j += blockSize)
		m_top[j] = reinterpret_cast< size_t >(&m_top[j + blockSize]);
	
	m_top[j] = 0;
	m_free = m_top;
}

void* BlockAllocator::top()
{
	return m_top;
}

void* BlockAllocator::alloc()
{
	void* p = 0;
	
	if (m_free)
	{
		p = m_free;
		m_free = reinterpret_cast< size_t* >(*m_free);
		T_ASSERT(m_free >= m_top && m_free < m_end || m_free == 0);
		
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
	
	T_ASSERT (((size_t*)p >= m_top && (size_t*)p < m_end) || p == 0);
	return p;
}

bool BlockAllocator::free(void* p)
{
	size_t* block = static_cast< size_t* >(p);
	if (block < m_top || block >= m_end)
		return false;
		
	*block = reinterpret_cast< size_t >(m_free);
	m_free = block;
	T_ASSERT(m_free >= m_top && m_free < m_end);

#if defined (_DEBUG)
	m_alloced--;
#endif
	return true;
}

bool BlockAllocator::belong(const void* p) const
{
	const size_t* block = static_cast< const size_t* >(p);
	return (block >= m_top && block < m_end);
}

}

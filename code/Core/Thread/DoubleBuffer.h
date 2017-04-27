/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_DoubleBuffer_H
#define traktor_DoubleBuffer_H

#include "Core/Thread/Atomic.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"

namespace traktor
{

/*! \brief Double buffer helper. 
 * \ingroup Core
 */
template < typename Type >
class DoubleBuffer
{
public:
	DoubleBuffer()
	:	m_lock(0)
	,	m_write(0)
	,	m_read(0)
	,	m_index(0)
	{
		m_data = new Type [2];
	}

	~DoubleBuffer()
	{
		delete[] m_data;
	}

	Type& beginWrite()
	{
		Thread* thread = ThreadManager::getInstance().getCurrentThread();
		while (Atomic::compareAndSwap(m_lock, 0, 1) != 1) { thread->yield(); }
		return m_data[m_index];
	}

	void endWrite()
	{
		Atomic::increment(m_write);
		m_lock = 0;
	}

	Type& read()
	{
		if (m_write != m_read)
		{
			Thread* thread = ThreadManager::getInstance().getCurrentThread();
			while (Atomic::compareAndSwap(m_lock, 0, 2) != 2) { thread->yield(); }
			m_index = 1 - m_index;
			m_read = m_write;
			m_lock = 0;
		}
		return m_data[1 - m_index];
	}

private:
	Type* m_data;
	int32_t m_lock;
	int32_t m_write;
	int32_t m_read;
	int32_t m_index;
};

}

#endif	// traktor_DoubleBuffer_H

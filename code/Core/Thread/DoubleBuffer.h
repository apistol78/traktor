/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Atomic.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"

namespace traktor
{

/*! Double buffer helper.
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


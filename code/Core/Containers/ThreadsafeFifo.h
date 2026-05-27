/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/SpinLock.h"

namespace traktor
{

/*!
 */
template < typename T >
class ThreadsafeFifo
{
public:
	void put(const T& item)
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		m_fifo.push_back(item);
	}

	bool get(T& item)
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		if (m_head < m_fifo.size())
		{
			// Pop from a moving head index instead of erasing the front element,
			// which would shift the entire vector (O(n)) on every dequeue.
			item = m_fifo[m_head];
			m_fifo[m_head] = T(); // Release the stored element promptly (matches erase semantics).
			++m_head;
			if (m_head >= m_fifo.size())
			{
				// Fully drained; reset to reuse storage from the front.
				m_fifo.resize(0);
				m_head = 0;
			}
			else if (m_head >= 64 && m_head * 2 >= m_fifo.size())
			{
				// Consumed prefix is large; compact once (amortized O(1)) to bound
				// memory when producers keep the queue non-empty.
				m_fifo.erase(m_fifo.begin(), m_fifo.begin() + m_head);
				m_head = 0;
			}
			return true;
		}
		else
			return false;
	}

	void clear()
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		m_fifo.clear();
		m_head = 0;
	}

private:
	SpinLock m_lock;
	AlignedVector< T > m_fifo;
	size_t m_head = 0;
};

}

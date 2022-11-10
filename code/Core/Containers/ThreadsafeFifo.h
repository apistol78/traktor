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
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

/*! Thread-safe FIFO queue.
 * \ingroup Core
 */
template < typename ItemType, typename LockType = Semaphore >
class ThreadsafeFifo
{
public:
	ThreadsafeFifo()
	:	m_size(0)
	{
	}

	void put(const ItemType& item)
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		m_items.push_back(item);
		m_size++;
	}

	bool get(ItemType& outItem)
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		if (!m_items.empty())
		{
			outItem = m_items.front();
			m_items.pop_front();
			m_size--;
			return true;
		}
		else
			return false;
	}

	void clear()
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		m_items.clear();
		m_size = 0;
	}

	bool empty() const
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		return m_items.empty();
	}

	uint32_t size() const
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		return m_size;
	}

private:
	mutable LockType m_lock;
	mutable uint32_t m_size;
	std::list< ItemType > m_items;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! Circular static container.
 *
 * \note
 * This implementation allocate Capacity+1 elements
 * so we can keep track of empty vs full.
 *
 * \ingroup Core
 */
template < typename ItemType, uint32_t Capacity >
class CircularVector
{
	static constexpr int N = Capacity + 1;

public:
	CircularVector()
	:	m_front(0)
	,	m_back(0)
	{
	}

	bool empty() const
	{
		return m_front == m_back;
	}

	bool full() const
	{
		return m_front == ((m_back + 1) % N);
	}

	uint32_t capacity() const
	{
		return Capacity;
	}

	uint32_t size() const
	{
		if (m_front <= m_back)
			return m_back - m_front;
		else
			return (N - m_front) + m_back;
	}

	void clear()
	{
		m_front = 0;
		m_back = 0;
	}

	ItemType& push_back()
	{
		T_ASSERT(!full());

		// Get back element.
		ItemType& item = m_items[m_back];
		m_back = (m_back + 1) % N;

		// Discard front element if we've reached front.
		if (m_back == m_front)
			m_front = (m_front + 1) % N;

		return item;
	}

	void push_back(const ItemType& item)
	{
		// Push back element.
		m_items[m_back] = item;
		m_back = (m_back + 1) % N;

		// Discard front element if we've reached front.
		if (m_back == m_front)
			m_front = (m_front + 1) % N;
	}

	void pop_back()
	{
		m_back = (m_back > 0) ? m_back - 1 : N - 1;
	}

	void pop_front()
	{
		T_ASSERT(!empty());
		m_front = (m_front + 1) % N;
	}

	int32_t find(const ItemType& value) const
	{
		const int32_t sz = (int32_t)size();
		for (int32_t i = 0; i < sz; ++i)
		{
			if ((*this)[i] == value)
				return i;
		}
		return -1;
	}

	const ItemType& back() const
	{
		T_ASSERT(!empty());
		return m_items[m_back > 0 ? (m_back - 1) : N - 1];
	}

	ItemType& back()
	{
		T_ASSERT(!empty());
		return m_items[m_back > 0 ? (m_back - 1) : N - 1];
	}

	const ItemType& front() const
	{
		T_ASSERT(!empty());
		return m_items[m_front];
	}

	ItemType& front()
	{
		T_ASSERT(!empty());
		return m_items[m_front];
	}

	const ItemType& offset(int32_t index) const
	{
		if (index < 0)
			return m_items[(m_back + index) % N];
		else
			return m_items[(m_front + index) % N];
	}

	const ItemType& operator [] (uint32_t index) const
	{
		return m_items[(index + m_front) % N];
	}

	ItemType& operator [] (uint32_t index)
	{
		return m_items[(index + m_front) % N];
	}

private:
	ItemType m_items[N];
	uint32_t m_front;
	uint32_t m_back;
};

}

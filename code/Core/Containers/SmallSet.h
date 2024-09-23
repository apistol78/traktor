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

namespace traktor
{

/*! Small "set" container.
 * \ingroup Core
 *
 * This container is optimized for a set
 * of a few unique items.
 */
template < typename Key >
class SmallSet
{
public:
	typedef typename AlignedVector< Key >::iterator iterator;
	typedef typename AlignedVector< Key >::const_iterator const_iterator;

	SmallSet() = default;

	SmallSet(const SmallSet& src)
	:	m_data(src.m_data)
	{
	}

	SmallSet(SmallSet&& src) noexcept
	{
		m_data = std::move(src.m_data);
	}

	SmallSet(const std::initializer_list< Key >& iv)
	:	m_data(iv)
	{
	}

	iterator begin()
	{
		return m_data.begin();
	}

	const_iterator begin() const
	{
		return m_data.begin();
	}

	iterator end()
	{
		return m_data.end();
	}

	const_iterator end() const
	{
		return m_data.end();
	}

	void clear()
	{
		m_data.clear();
	}

	void reset()
	{
		m_data.resize(0);
	}

	bool empty() const
	{
		return m_data.empty();
	}

	iterator find(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return m_data.begin() + i;
		}

		return m_data.end();
	}

	const_iterator find(const Key& value) const
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return m_data.begin() + i;
		}

		return m_data.end();
	}

	bool insert(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return false;
		}

		T_ASSERT(is <= m_data.size());
		m_data.insert(m_data.begin() + is, value);
		return true;
	}

	uint32_t insert(const const_iterator& from, const const_iterator& to)
	{
		uint32_t inserted = 0;
		for (const_iterator i = from; i != to; ++i)
		{
			if (insert(*i))
				++inserted;
		}
		return inserted;
	}

	void erase(const iterator& at)
	{
		m_data.erase(at);
	}

	bool erase(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
			{
				m_data.erase(m_data.begin() + i);
				return true;
			}
		}

		return false;
	}

	size_t size() const
	{
		return m_data.size();
	}

	const Key& operator [] (size_t index) const
	{
		return m_data[index];
	}

	SmallSet& operator = (const SmallSet& src)
	{
		m_data = src.m_data;
		return *this;
	}

	SmallSet& operator = (SmallSet&& src) noexcept
	{
		m_data = std::move(src.m_data);
		return *this;
	}

private:
	AlignedVector< Key > m_data;
};

}

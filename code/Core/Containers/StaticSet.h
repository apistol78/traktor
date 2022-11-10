/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/StaticVector.h"

namespace traktor
{

/*! Small static "set" container.
 * \ingroup Core
 *
 * This container is optimized for a set
 * of a few unique items.
 */
template < typename Key, size_t Capacity >
class StaticSet
{
public:
	typedef typename StaticVector< Key, Capacity >::iterator iterator;
	typedef typename StaticVector< Key, Capacity >::const_iterator const_iterator;

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
			size_t i = (is + ie) >> 1;
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
			size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return m_data.begin() + i;
		}

		return m_data.end();
	}

	size_t count(const Key& value) const
	{
		return find(value) != end() ? 1 : 0;
	}

	bool insert(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
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

	template < typename IteratorType >
	bool insert(const IteratorType& from, const IteratorType& to)
	{
		for (IteratorType i = from; i != to; ++i)
		{
			if (!insert(*i))
				return false;
		}
		return true;
	}

	bool erase(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
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

	void erase(const iterator& where)
	{
		m_data.erase(where);
	}

	void erase(const const_iterator& where)
	{
		m_data.erase(where);
	}

	size_t size() const
	{
		return m_data.size();
	}

	const Key& operator [] (size_t index) const
	{
		return m_data[index];
	}

	bool operator == (const StaticSet< Key, Capacity >& rh) const
	{
		return m_data == rh.m_data;
	}

	bool operator != (const StaticSet< Key, Capacity >& rh) const
	{
		return m_data != rh.m_data;
	}

private:
	StaticVector< Key, Capacity > m_data;
};

}

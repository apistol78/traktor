/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <utility>
#include "Core/Containers/AlignedVector.h"

namespace traktor
{

/*! \ingroup Core */
template < typename Key >
struct SmallMapKeyView
{
	typedef const Key& view_type_t;
};

/*! \ingroup Core */
template < >
struct SmallMapKeyView < std::string >
{
	typedef const std::string_view& view_type_t;
};

/*! \ingroup Core */
template < >
struct SmallMapKeyView < std::wstring >
{
	typedef const std::wstring_view& view_type_t;
};

/*! Small directional map
 * \ingroup Core
 *
 * This container is optimized for fast lookup
 * of a small, ~10-20, set of items.
 * Lookup is performed in a sorted, linear
 * array of pairs using simple binary search.
 */
template < typename Key, typename Item >
class SmallMap
{
public:
	typedef std::pair< Key, Item > pair_t;
	typedef typename AlignedVector< pair_t >::iterator iterator;
	typedef typename AlignedVector< pair_t >::const_iterator const_iterator;
	typedef typename AlignedVector< pair_t >::reverse_iterator reverse_iterator;
	typedef typename AlignedVector< pair_t >::const_reverse_iterator const_reverse_iterator;
	typedef typename SmallMapKeyView< Key >::view_type_t view_type_t;

	SmallMap() = default;

	SmallMap(const SmallMap& src)
	:	m_data(src.m_data)
	{
	}

	SmallMap(SmallMap&& src) noexcept
	{
		m_data = std::move(src.m_data);
	}

	void swap(SmallMap& src)
	{
		m_data.swap(src.m_data);
	}

	void reserve(size_t capacity)
	{
		m_data.reserve(capacity);
	}

	iterator begin()
	{
		return m_data.begin();
	}

	const_iterator begin() const
	{
		return m_data.begin();
	}

	reverse_iterator rbegin()
	{
		return m_data.rbegin();
	}

	const_reverse_iterator rbegin() const
	{
		return m_data.rbegin();
	}

	iterator end()
	{
		return m_data.end();
	}

	const_iterator end() const
	{
		return m_data.end();
	}

	reverse_iterator rend()
	{
		return m_data.rend();
	}

	const_reverse_iterator rend() const
	{
		return m_data.rend();
	}

	iterator find(const view_type_t& key)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data.begin() + i;
		}

		return end();
	}

	const_iterator find(const view_type_t& key) const
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data.begin() + i;
		}

		return end();
	}

	void insert(const pair_t& pair)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (pair.first < m_data[i].first)
				ie = i;
			else if (pair.first > m_data[i].first)
				is = i + 1;
			else if (pair.first == m_data[i].first)
				return;
		}

		T_ASSERT(is <= m_data.size());
		m_data.insert(m_data.begin() + is, pair);
	}

	void insert(const view_type_t& key, const Item& item)
	{
		insert(std::make_pair(Key(key), item));
	}

	void insert(const const_iterator& first, const const_iterator& last)
	{
		for (const_iterator i = first; i != last; ++i)
			insert(*i);
	}

	iterator erase(const iterator& it)
	{
		return m_data.erase(it);
	}

	iterator erase(const iterator& first, const iterator& last)
	{
		return m_data.erase(first, last);
	}

	bool remove(const view_type_t& key)
	{
		iterator it = find(key);
		if (it != end())
		{
			erase(it);
			return true;
		}
		else
			return false;
	}

	/*! Reset content but keep memory allocated.
	 * This method reset the map's content but keeps memory
	 * allocated for it's internal vector to quickly be
	 * able to rebuild map without reallocating memory.
	 */
	void reset()
	{
		m_data.resize(0);
	}

	/*! Clear content and release allocated memory. */
	void clear()
	{
		m_data.clear();
	}

	bool empty() const
	{
		return m_data.empty();
	}

	size_t size() const
	{
		return m_data.size();
	}

	Item& operator [] (const view_type_t& key)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data[i].second;
		}

		m_data.insert(m_data.begin() + is, std::make_pair(Key(key), Item()));
		return m_data[is].second;
	}

	Item operator [] (const view_type_t& key) const
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			const size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data[i].second;
		}

		return Item();
	}

	SmallMap& operator = (const SmallMap& src)
	{
		m_data = src.m_data;
		return *this;
	}

	SmallMap& operator = (SmallMap&& src) noexcept
	{
		m_data = std::move(src.m_data);
		return *this;
	}

private:
	AlignedVector< pair_t > m_data;
};

}

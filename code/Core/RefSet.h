/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include "Core/Ref.h"
#include "Core/Containers/SmallSet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Object set reference container.
 * \ingroup Core
 */
template < typename Class >
class RefSet
{
public:
	typedef SmallSet< Class* > set_t;
	typedef typename set_t::const_iterator const_iterator;

	RefSet() = default;

	RefSet(const RefSet< Class >& rs)
	:	m_items(rs.m_items)
	{
		for (auto item : m_items)
			T_SAFE_ADDREF(item);
	}

	RefSet(RefSet< Class >&& rs)
	:	m_items(rs.m_items)
	{
		m_items = std::move(rs.m_items);
	}

	virtual ~RefSet()
	{
		clear();
	}

	void clear()
	{
		for (auto item : m_items)
			T_SAFE_RELEASE(item);
		m_items.clear();
	}

	void reset()
	{
		for (auto item : m_items)
			T_SAFE_RELEASE(item);
		m_items.reset();
	}

	bool empty() const
	{
		return m_items.empty();
	}

	size_t size() const
	{
		return m_items.size();
	}

	bool insert(Class* item)
	{
		T_ASSERT(item);
		if (m_items.insert(item))
		{
			T_SAFE_ADDREF(item);
			return true;
		}
		else
			return false;
	}

	bool erase(Class* item)
	{
		T_ASSERT(item);
		if (m_items.erase(item))
		{
			T_SAFE_RELEASE(item);
			return true;
		}
		else
			return false;
	}

	const_iterator find(Class* item) const
	{
		return m_items.find(item);
	}

	const_iterator begin() const
	{
		return m_items.begin();
	}

	const_iterator end() const
	{
		return m_items.end();
	}

	RefSet< Class >& operator = (const RefSet< Class >& rs)
	{
		reset();
		m_items = rs.m_items;
		for (auto item : m_items)
			T_SAFE_ADDREF(item);
		return *this;
	}

	RefSet< Class >& operator = (RefSet< Class >&& rs)
	{
		reset();
		m_items = std::move(rs.m_items);
		return *this;
	}

private:
	set_t m_items;
};

}


/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_RefSet_H
#define traktor_RefSet_H

#include <algorithm>
#include <set>
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Object set reference container.
 * \ingroup Core
 */
template < typename Class >
class RefSet
{
public:
	typedef std::set< Class* > set_t;
	typedef typename set_t::const_iterator const_iterator;

	RefSet()
	{
	}

	RefSet(const RefSet< Class >& rs)
	:	m_items(rs.m_items)
	{
		for (typename set_t::iterator i = m_items.begin(); i != m_items.end(); ++i)
			T_SAFE_ADDREF(*i);
	}

	virtual ~RefSet()
	{
		clear();
	}

	void clear()
	{
		for (typename set_t::iterator i = m_items.begin(); i != m_items.end(); ++i)
			T_SAFE_RELEASE(*i);
		m_items.clear();
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
		T_ASSERT (item);
		std::pair< typename set_t::iterator, bool > ins = m_items.insert(item);
		if (ins.second)
		{
			T_SAFE_ADDREF(item);
			return true;
		}
		else
			return false;
	}

	size_t erase(Class* item)
	{
		T_ASSERT (item);
		size_t x = m_items.erase(item);
		if (x)
			T_SAFE_RELEASE(item);
		return x;
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
		clear();
		m_items = rs.m_items;
		for (typename set_t::iterator i = m_items.begin(); i != m_items.end(); ++i)
			T_SAFE_ADDREF(*i);
		return *this;
	}

private:
	set_t m_items;
};

}

#endif	// traktor_RefSet_H

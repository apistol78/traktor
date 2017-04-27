/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_InstrusiveList_H
#define traktor_InstrusiveList_H

#include <iterator>
#include "Core/Config.h"

namespace traktor
{

/*! \brief Default link accessors.
 * \ingroup Core
 */
template < typename ItemType >
struct DefaultLink
{
	ItemType* m_item;

	DefaultLink(ItemType* item)
	:	m_item(item)
	{
	}

	ItemType*& prev()
	{
		return m_item->m_prev;
	}

	ItemType*& next()
	{
		return m_item->m_next;
	}
};

/*! \brief Intrusive double linked list.
 * \ingroup Core
 *
 * Intrusive means it uses members in elements
 * to maintain links.
 */
template <
	typename ItemType,
	typename L = DefaultLink< ItemType >
>
class IntrusiveList
{
public:
	class const_iterator
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef const value_type* pointer;

		pointer operator * () const
		{
			return m_current;
		}

		pointer operator -> () const
		{
			return m_current;
		}

		const_iterator& operator ++ ()
		{
			T_ASSERT (m_current);
			m_current = L(m_current).next();
			return *this;
		}

		const_iterator operator ++ (int)
		{
			T_ASSERT (m_current);
			iterator it(m_list, m_current);
			m_current = L(m_current).next();
			return it;
		}

		const_iterator& operator -- ()
		{
			T_ASSERT (m_current);
			m_current = L(m_current).prev();
			return *this;
		}

		const_iterator operator -- (int)
		{
			T_ASSERT (m_current);
			iterator it(m_list, m_current);
			m_current = L(m_current).prev();
			return it;
		}

		bool operator == (const const_iterator& r) const
		{
			return m_current == r.m_current;
		}

		bool operator != (const const_iterator& r) const
		{
			return m_current != r.m_current;
		}

	protected:
		friend class IntrusiveList;
		const IntrusiveList* m_list;
		value_type* m_current;

		explicit const_iterator(const IntrusiveList* list, ItemType* current)
		:	m_list(list)
		,	m_current(current)
		{
		}
	};

	class iterator : public const_iterator
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const_iterator _O;

		pointer operator * () const
		{
			return _O::m_current;
		}

		pointer operator -> () const
		{
			return _O::m_current;
		}

		iterator& operator ++ ()
		{
			T_ASSERT (_O::m_current);
			_O::m_current = L(_O::m_current).next();
			return *this;
		}

		iterator operator ++ (int)
		{
			T_ASSERT (_O::m_current);
			iterator it(_O::m_list, _O::m_current);
			_O::m_current = L(_O::m_current).next();
			return it;
		}

		iterator& operator -- ()
		{
			T_ASSERT (_O::m_current);
			_O::m_current = L(_O::m_current).prev();
			return *this;
		}

		iterator operator -- (int)
		{
			T_ASSERT (_O::m_current);
			iterator it(_O::m_list,_O:: m_current);
			_O::m_current = L(_O::m_current).prev();
			return it;
		}

		bool operator == (const iterator& r) const
		{
			return _O::m_current == r.m_current;
		}

		bool operator != (const iterator& r) const
		{
			return _O::m_current != r.m_current;
		}

	protected:
		friend class IntrusiveList;

		explicit iterator(const IntrusiveList* list, ItemType* current)
		:	const_iterator(list, current)
		{
		}
	};

	IntrusiveList()
	:	m_front(0)
	,	m_back(0)
	{
	}

	bool empty() const
	{
		return m_front == 0;
	}

	void push_front(ItemType* item)
	{
		T_ASSERT (L(item).prev() == 0);
		T_ASSERT (L(item).next() == 0);

		if (m_front)
		{
			T_ASSERT (item != m_front);

			L(item).prev() = 0;
			L(item).next() = m_front;

			T_ASSERT (L(m_front).prev() == 0);
			L(m_front).prev() = item;
		}
		else
		{
			T_ASSERT (!m_back);
			m_back = item;
		}
		m_front = item;

		T_ASSERT (L(m_front).prev() == 0);
	}

	void push_back(ItemType* item)
	{
		T_ASSERT (L(item).prev() == 0);
		T_ASSERT (L(item).next() == 0);

		if (m_back)
		{
			T_ASSERT (item != m_back);

			L(item).prev() = m_back;
			L(item).next() = 0;

			T_ASSERT (L(m_back).next() == 0);
			L(m_back).next() = item;
		}
		else
		{
			T_ASSERT (!m_front);
			m_front = item;
		}
		m_back = item;

		T_ASSERT (L(m_back).next() == 0);
	}

	void pop_front()
	{
		T_ASSERT (m_front);

		ItemType* front = L(m_front).next();
		if (front)
			L(front).prev() = 0;

		L(m_front).prev() =
		L(m_front).next() = 0;

		if ((m_front = front) == 0)
			m_back = 0;
	}

	ItemType* front()
	{
		return m_front;
	}

	ItemType* back()
	{
		return m_back;
	}

	iterator begin()
	{
		return iterator(this, m_front);
	}

	iterator end()
	{
		return iterator(this, 0);
	}

	const_iterator begin() const
	{
		return const_iterator(this, m_front);
	}

	const_iterator end() const
	{
		return const_iterator(this, 0);
	}

	iterator erase(const iterator& it)
	{
		T_ASSERT (it.m_list == this);
		iterator it2(this, L(it.m_current).next());
		remove(it.m_current);
		return it2;
	}

	void remove(ItemType* item)
	{
		ItemType* prev = L(item).prev();
		ItemType* next = L(item).next();

		if (prev)
		{
			T_ASSERT (L(prev).next() == item);
			L(prev).next() = next;
		}
		else
		{
			T_ASSERT (item == m_front);
			m_front = next;
			if (m_front)
				L(m_front).prev() = 0;
		}

		if (next)
			L(next).prev() = prev;
		else
		{
			T_ASSERT (item == m_back);
			m_back = prev;
			if (m_back)
				L(m_back).next() = 0;
		}

		L(item).prev() =
		L(item).next() = 0;
	}

private:
	ItemType* m_front;
	ItemType* m_back;
};

}

#endif	// traktor_InstrusiveList_H

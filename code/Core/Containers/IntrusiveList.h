/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <iterator>
#include "Core/Config.h"

namespace traktor
{

/*! Default link accessors.
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

/*! Intrusive double linked list.
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
			T_ASSERT(m_current);
			m_current = L(m_current).next();
			return *this;
		}

		const_iterator operator ++ (int)
		{
			T_ASSERT(m_current);
			iterator it(m_list, m_current);
			m_current = L(m_current).next();
			return it;
		}

		const_iterator& operator -- ()
		{
			T_ASSERT(m_current);
			m_current = L(m_current).prev();
			return *this;
		}

		const_iterator operator -- (int)
		{
			T_ASSERT(m_current);
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
			T_ASSERT(_O::m_current);
			_O::m_current = L(_O::m_current).next();
			return *this;
		}

		iterator operator ++ (int)
		{
			T_ASSERT(_O::m_current);
			iterator it(_O::m_list, _O::m_current);
			_O::m_current = L(_O::m_current).next();
			return it;
		}

		iterator& operator -- ()
		{
			T_ASSERT(_O::m_current);
			_O::m_current = L(_O::m_current).prev();
			return *this;
		}

		iterator operator -- (int)
		{
			T_ASSERT(_O::m_current);
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

	bool empty() const
	{
		return m_front == nullptr;
	}

	void push_front(ItemType* item)
	{
		T_ASSERT(L(item).prev() == nullptr);
		T_ASSERT(L(item).next() == nullptr);

		if (m_front)
		{
			T_ASSERT(item != m_front);

			L(item).prev() = nullptr;
			L(item).next() = m_front;

			T_ASSERT(L(m_front).prev() == nullptr);
			L(m_front).prev() = item;
		}
		else
		{
			T_ASSERT(!m_back);
			m_back = item;
		}
		m_front = item;

		T_ASSERT(L(m_front).prev() == nullptr);
	}

	void push_back(ItemType* item)
	{
		T_ASSERT(L(item).prev() == nullptr);
		T_ASSERT(L(item).next() == nullptr);

		if (m_back)
		{
			T_ASSERT(item != m_back);

			L(item).prev() = m_back;
			L(item).next() = nullptr;

			T_ASSERT(L(m_back).next() == nullptr);
			L(m_back).next() = item;
		}
		else
		{
			T_ASSERT(!m_front);
			m_front = item;
		}
		m_back = item;

		T_ASSERT(L(m_back).next() == nullptr);
	}

	void pop_front()
	{
		T_ASSERT(m_front);

		ItemType* front = L(m_front).next();
		if (front)
			L(front).prev() = nullptr;

		L(m_front).prev() =
		L(m_front).next() = nullptr;

		if ((m_front = front) == nullptr)
			m_back = nullptr;
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
		return iterator(this, nullptr);
	}

	const_iterator begin() const
	{
		return const_iterator(this, m_front);
	}

	const_iterator end() const
	{
		return const_iterator(this, nullptr);
	}

	iterator erase(const iterator& it)
	{
		T_ASSERT(it.m_list == this);
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
			T_ASSERT(L(prev).next() == item);
			L(prev).next() = next;
		}
		else
		{
			T_ASSERT(item == m_front);
			m_front = next;
			if (m_front)
				L(m_front).prev() = nullptr;
		}

		if (next)
			L(next).prev() = prev;
		else
		{
			T_ASSERT(item == m_back);
			m_back = prev;
			if (m_back)
				L(m_back).next() = nullptr;
		}

		L(item).prev() =
		L(item).next() = nullptr;
	}

private:
	ItemType* m_front = nullptr;
	ItemType* m_back = nullptr;
};

}

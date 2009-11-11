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
	{
	}

	bool empty() const
	{
		sanityCheck();
		return m_front == 0;
	}

	void push_front(ItemType* item)
	{
		sanityCheck();

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
		m_front = item;

		T_ASSERT (L(m_front).prev() == 0);

		sanityCheck();
	}

	void pop_front()
	{
		sanityCheck();

		T_ASSERT (m_front);

		ItemType* front = L(m_front).next();
		if (front)
			L(front).prev() = 0;

		L(m_front).prev() =
		L(m_front).next() = 0;

		m_front = front;

		sanityCheck();
	}

	ItemType* front()
	{
		return m_front;
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

		sanityCheck();

		iterator it2(this, L(it.m_current).next());
		remove(it.m_current);

		sanityCheck();

		return it2;
	}

	void remove(ItemType* item)
	{
		sanityCheck();

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

		L(item).prev() =
		L(item).next() = 0;

		sanityCheck();
	}

private:
	ItemType* m_front;

	void sanityCheck() const
	{
#if defined(_DEBUG)
		ItemType* prev = 0;
		for (ItemType* item = m_front; item; )
		{
			ItemType* P = L(item).prev();
			T_FATAL_ASSERT (P == prev);

			ItemType* N = L(item).next();
			T_FATAL_ASSERT (N != item);

			if (N != 0 && prev != 0)
				T_FATAL_ASSERT (N != prev);

			prev = item;
			item = N;
		}
#endif
	}
};

}

#endif	// traktor_InstrusiveList_H

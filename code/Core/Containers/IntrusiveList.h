#ifndef traktor_InstrusiveList_H
#define traktor_InstrusiveList_H

#include <iterator>
#include "Core/Config.h"

namespace traktor
{

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

	/*! \brief Intrusive double linked list. */
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
			typedef typename int difference_type;
			typedef typename const ItemType* pointer;

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
			ItemType* m_current;

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
			typedef typename int difference_type;
			typedef typename ItemType* pointer;

			pointer operator * () const
			{
				return m_current;
			}

			pointer operator -> () const
			{
				return m_current;
			}

			iterator& operator ++ ()
			{
				T_ASSERT (m_current);
				m_current = L(m_current).next();
				return *this;
			}

			iterator operator ++ (int)
			{
				T_ASSERT (m_current);
				iterator it(m_list, m_current);
				m_current = L(m_current).next();
				return it;
			}

			iterator& operator -- ()
			{
				T_ASSERT (m_current);
				m_current = L(m_current).prev();
				return *this;
			}

			iterator operator -- (int)
			{
				T_ASSERT (m_current);
				iterator it(m_list, m_current);
				m_current = L(m_current).prev();
				return it;
			}

			bool operator == (const iterator& r) const
			{
				return m_current == r.m_current;
			}

			bool operator != (const iterator& r) const
			{
				return m_current != r.m_current;
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
			return m_front == 0;
		}

		void push_front(ItemType* item)
		{
			T_ASSERT (L(item).prev() == 0);
			T_ASSERT (L(item).next() == 0);
			if (m_front)
			{
				L(item).prev() = 0;
				L(item).next() = m_front;

				T_ASSERT (L(m_front).prev() == 0);
				L(m_front).prev() = item;
			}
			m_front = item;
			T_ASSERT (L(m_front).prev() == 0);
		}

		void pop_front()
		{
			T_ASSERT (m_front);

			ItemType* front = L(m_front).next();
			if (front)
				L(front).prev() = 0;

			L(m_front).prev() =
			L(m_front).next() = 0;

			m_front = front;
		}

		ItemType* front()
		{
			T_ASSERT (m_front);
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

		void erase(const iterator& it)
		{
			T_ASSERT (it.m_list == this);
			remove(it.m_current);
		}

		void remove(ItemType* item)
		{
			if (L(item).prev())
				L(L(item).prev()).next() = L(item).next();
			else
			{
				T_ASSERT (item == m_front);
				m_front = L(item).next();
				if (m_front)
					L(m_front).prev() = 0;
			}
			if (L(item).next())
				L(L(item).next()).prev() = L(item).prev();

			L(item).prev() =
			L(item).next() = 0;
		}

	private:
		ItemType* m_front;
	};

}

#endif	// traktor_InstrusiveList_H

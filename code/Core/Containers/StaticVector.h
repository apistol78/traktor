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
#include "Core/Containers/VectorConstructor.h"

namespace traktor
{

/*! Static vector container.
 * \ingroup Core
 */
template < typename ItemType, size_t Capacity_, typename Constructor = VectorConstructor< ItemType > >
class StaticVector
{
public:
	typedef ItemType value_type;
	typedef int difference_type;
	typedef const value_type* const_pointer;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef value_type& reference;

	class const_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef const value_type* pointer;
		typedef const value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		const_iterator() = default;

		const_iterator(const const_iterator& r)
		:	m_ptr(r.m_ptr)
		{
		}

		reference operator * () const
		{
			return *m_ptr;
		}

		pointer operator -> () const
		{
			return m_ptr;
		}

		const_iterator operator + (int offset) const
		{
			return const_iterator(m_ptr + offset);
		}

		const_iterator operator + (size_t offset) const
		{
			return const_iterator(m_ptr + offset);
		}

		const_iterator operator - (int offset) const
		{
			return const_iterator(m_ptr - offset);
		}

		const_iterator operator - (size_t offset) const
		{
			return const_iterator(m_ptr - offset);
		}

		void operator += (int offset)
		{
			m_ptr += offset;
		}

		void operator -= (int offset)
		{
			m_ptr -= offset;
		}

		const_iterator operator ++ ()		// pre-fix
		{
			return const_iterator(++m_ptr);
		}

		const_iterator operator ++ (int)	// post-fix
		{
			return const_iterator(m_ptr++);
		}

		const_iterator operator -- ()
		{
			return const_iterator(--m_ptr);
		}

		const_iterator operator -- (int)
		{
			return const_iterator(m_ptr--);
		}

		bool operator == (const const_iterator& r) const
		{
			return m_ptr == r.m_ptr;
		}

		bool operator != (const const_iterator& r) const
		{
			return m_ptr != r.m_ptr;
		}

		bool operator < (const const_iterator& r) const
		{
			return m_ptr < r.m_ptr;
		}

		bool operator > (const const_iterator& r) const
		{
			return m_ptr > r.m_ptr;
		}

		bool operator >= (const const_iterator& r) const
		{
			return m_ptr >= r.m_ptr;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(m_ptr - r.m_ptr);
		}

		const_iterator& operator = (const const_iterator& r)
		{
			m_ptr = r.m_ptr;
			return *this;
		}

		const_pointer _const_ptr() const
		{
			return m_ptr;
		}

	protected:
		friend class StaticVector;
		ItemType* m_ptr = nullptr;

		explicit const_iterator(const ItemType* ptr)
		:	m_ptr(const_cast< ItemType* >(ptr))
		{
		}
	};

	class iterator : public const_iterator
	{
	public:
		typedef const_iterator _O;
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		iterator() : const_iterator()
		{
		}

		reference operator * () const
		{
			return *_O::m_ptr;
		}

		pointer operator -> () const
		{
			return _O::m_ptr;
		}

		reference operator [] (int offset) const
		{
			return *(_O::m_ptr + offset);
		}

		iterator operator + (int offset) const
		{
			return iterator(_O::m_ptr + offset);
		}

		iterator operator + (size_t offset) const
		{
			return iterator(_O::m_ptr + offset);
		}

		iterator operator - (int offset) const
		{
			return iterator(_O::m_ptr - offset);
		}

		iterator operator - (size_t offset) const
		{
			return iterator(_O::m_ptr - offset);
		}

		iterator operator ++ ()		// pre-fix
		{
			return iterator(++_O::m_ptr);
		}

		iterator operator ++ (int)	// post-fix
		{
			return iterator(_O::m_ptr++);
		}

		iterator operator -- ()
		{
			return iterator(--_O::m_ptr);
		}

		iterator operator -- (int)
		{
			return iterator(_O::m_ptr--);
		}

		bool operator == (const iterator& r) const
		{
			return _O::m_ptr == r.m_ptr;
		}

		bool operator != (const iterator& r) const
		{
			return _O::m_ptr != r.m_ptr;
		}

		bool operator < (const iterator& r) const
		{
			return _O::m_ptr < r.m_ptr;
		}

		bool operator > (const iterator& r) const
		{
			return _O::m_ptr > r.m_ptr;
		}

		bool operator >= (const iterator& r) const
		{
			return _O::m_ptr >= r.m_ptr;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(_O::m_ptr - r._const_ptr());
		}

	protected:
		friend class StaticVector;

		explicit iterator(ItemType* ptr)
		:	const_iterator(ptr)
		{
		}
	};

	enum { Capacity = Capacity_ };

	StaticVector() = default;

	explicit StaticVector(size_t size)
	:	m_size(size)
	{
		T_ASSERT(m_size < Capacity);
	}

	explicit StaticVector(size_t size, const_reference init)
	:	m_size(0)
	{
		resize(size, init);
	}

	template < typename IteratorType >
	explicit StaticVector(const IteratorType& from, const IteratorType& to)
	:	m_size(0)
	{
		for (IteratorType i = from; i != to; ++i)
			push_back(*i);
	}

	/*! Get number of elements in vector.
	 *
	 * \return Number of elements.
	 */
	size_t size() const
	{
		return m_size;
	}

	/*! Set number of elements in vector.
	 *
	 * \param size New number of elements.
	 */
	void resize(size_t size, const_reference init = value_type())
	{
		T_ASSERT(size <= Capacity);
		if (size > m_size)
		{
			for (size_t i = m_size; i < size; ++i)
				Constructor::construct(m_items[i], init);
		}
		else if (size < m_size)
		{
			for (size_t i = size; i < m_size; ++i)
				Constructor::destroy(m_items[i]);
		}
		m_size = size;
	}

	/*! Get number of elements allocated by vector.
	 *
	 * \return Number of allocated elements.
	 */
	size_t capacity() const
	{
		return Capacity;
	}

	/*! Check if vector is empty.
	 *
	 * \return True if vector empty.
	 */
	bool empty() const
	{
		return m_size == 0;
	}

	/*! Check if vector is full.
	 *
	 * \return True if vector full.
	 */
	bool full() const
	{
		return m_size >= Capacity;
	}

	/*! Clear vector. */
	void clear()
	{
		m_size = 0;
	}

	/*! Assign value to vector.
	 *
	 * \param size Size of vector.
	 * \param value Value of each element.
	 */
	void assign(size_t size, const ItemType& value)
	{
		T_ASSERT(size < Capacity);
		m_size = size;
		for (size_t i = 0; i < m_size; ++i)
			Constructor::construct(m_items[i], value);
	}

	/*! Push value onto vector.
	 *
	 * \return Item value.
	 */
	ItemType& push_back()
	{
		T_ASSERT(m_size < Capacity);
		m_size++;
		Constructor::construct(m_items[m_size - 1]);
		return m_items[m_size - 1];
	}

	/*! Push value onto vector.
	 *
	 * \param item Item value.
	 */
	ItemType& push_back(const ItemType& item)
	{
		T_ASSERT(m_size < Capacity);
		Constructor::construct(m_items[m_size++], item);
		return m_items[m_size - 1];
	}

	/*! Pop value from vector. */
	void pop_back()
	{
		T_ASSERT(m_size > 0);
		Constructor::destroy(m_items[m_size - 1]);
		--m_size;
	}

	/*! Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& front()
	{
		T_ASSERT(m_size > 0);
		return m_items[0];
	}

	/*! Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& front() const
	{
		T_ASSERT(m_size > 0);
		return m_items[0];
	}

	/*! Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& back()
	{
		T_ASSERT(m_size > 0);
		return m_items[m_size - 1];
	}

	/*! Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& back() const
	{
		T_ASSERT(m_size > 0);
		return m_items[m_size - 1];
	}

	/*! Return iterator at first element.
	 *
	 * \return Iterator.
	 */
	iterator begin()
	{
		return iterator(m_items);
	}

	/*! Return iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	iterator end()
	{
		return iterator(&m_items[m_size]);
	}

	/*! Return constant iterator at first element.
	 *
	 * \return Iterator.
	 */
	const_iterator begin() const
	{
		return const_iterator(m_items);
	}

	/*! Return constant iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	const_iterator end() const
	{
		return const_iterator(&m_items[m_size]);
	}

	/*! Insert element into vector.
	 *
	 * \param where Iterator at element.
	 * \param item Item value.
	 * \return Iterator at new element.
	 */
	iterator insert(const iterator& where, const ItemType& item)
	{
		const size_t size = m_size;
		const size_t offset = size_t(where.m_ptr - &m_items[0]);

		m_size++;
		T_ASSERT(m_size <= Capacity);

		// Initialize grown item.
		Constructor::construct(m_items[size]);

		// Move items to make space for new item.
		for (size_t i = size; i > offset; --i)
			move(i, i - 1);

		// Copy insert item into location.
		Constructor::construct(m_items[offset], item);

		return iterator(&m_items[offset]);
	}

	/*! Insert elements into vector.
	 *
	 * \param where Iterator at element.
	 * \param from Iterator at first insert element.
	 * \param to Iterator at last insert element.
	 * \return Iterator at new element.
	 */
	template< typename IteratorType >
	iterator insert(const iterator& where, const IteratorType& from, const IteratorType& to)
	{
		auto fptr = &(*from);
		auto tptr = &(*to);

		const size_t size = m_size;
		const size_t offset = size_t(where.m_ptr - m_items);
		const size_t count = size_t(tptr - fptr);

		m_size += count;
		T_ASSERT(m_size <= Capacity);

		// Initialize grown items.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_items[i + size]);

		// Move items to make room for items to be inserted.
		const int32_t mv = (int32_t)(size - offset);
		for (int32_t i = mv - 1; i >= 0; --i)
		{
			T_ASSERT(i + offset < size);
			T_ASSERT(i + offset + count < m_size);
			move(i + offset + count, i + offset);
		}

		// Copy insert items into location.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_items[i + offset], fptr[i]);

		return iterator(&m_items[offset]);
	}

	/*! Insert elements into vector.
	 *
	 * \param where Iterator at element.
	 * \param from Iterator at first insert element.
	 * \param to Iterator at last insert element.
	 * \return Iterator at new element.
	 */
	iterator insert(const iterator& where, const ItemType* from, const ItemType* to)
	{
		const size_t size = m_size;
		const size_t offset = size_t(where.m_ptr - m_items);
		const size_t count = size_t(to - from);

		m_size += count;
		T_ASSERT(m_size <= Capacity);

		// Initialize grown items.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_items[i + size]);

		// Move items to make room for items to be inserted.
		const int32_t mv = (int32_t)(size - offset);
		for (int32_t i = mv - 1; i >= 0; --i)
		{
			T_ASSERT(i + offset < size);
			T_ASSERT(i + offset + count < m_size);
			move(i + offset + count, i + offset);
		}

		// Copy insert items into location.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_items[i + offset], from[i]);

		return iterator(&m_items[offset]);
	}

	/*! Erase element.
	 *
	 * \param where Iterator at element.
	 * \return New iterator at next element.
	 */
	iterator erase(const iterator& where)
	{
		T_ASSERT(m_size > 0);
		const size_t offset = size_t(where.m_ptr - m_items);

		for (size_t i = offset; i < m_size - 1; ++i)
			move(i, i + 1);

		Constructor::destroy(m_items[m_size - 1]);
		m_size--;

		return iterator(&m_items[offset]);
	}

	/*! Erase range of elements.
	 *
	 * \param where Iterator at element.
	 * \param last Iterator to last element.
	 * \return New iterator at next element.
	 */
	iterator erase(const iterator& where, const iterator& last)
	{
		T_ASSERT(where.m_ptr <= last.m_ptr);

		const size_t offset = size_t(where.m_ptr - m_items);
		const size_t count = size_t(last.m_ptr - where.m_ptr);
		T_ASSERT(count <= m_size);

		if (count > 0)
		{
			for (size_t i = offset; i < m_size - count; ++i)
				move(i, i + count);

			for (size_t i = m_size - count; i < m_size; ++i)
				Constructor::destroy(m_items[i]);

			m_size -= count;
		}

		return iterator(&m_items[offset]);
	}

	/*!
	 */
	ItemType* ptr()
	{
		return m_items;
	}

	/*!
	 */
	const ItemType* c_ptr() const
	{
		return m_items;
	}

	/*!
	 */
	ItemType& operator [] (size_t index)
	{
		T_ASSERT(index < Capacity);
		return m_items[index];
	}

	/*!
	 */
	const ItemType& operator [] (size_t index) const
	{
		T_ASSERT(index < Capacity);
		return m_items[index];
	}

	/*!
	 */
	StaticVector& operator = (const StaticVector& src)
	{
		resize(0);
		insert(begin(), src.begin(), src.end());
		return *this;
	}

	/*!
	 */
	bool operator == (const StaticVector& rh) const
	{
		if (m_size != rh.m_size)
			return false;

		for (size_t i = 0; i < m_size; ++i)
		{
			if (m_items[i] != rh.m_items[i])
				return false;
		}

		return true;
	}

	/*!
	 */
	bool operator != (const StaticVector& rh) const
	{
		return !(*this == rh);
	}

private:
	size_t m_size = 0;
	ItemType m_items[Capacity];

	void move(size_t target, size_t source)
	{
		Constructor::move(m_items[target], m_items[source]);
	}
};

}

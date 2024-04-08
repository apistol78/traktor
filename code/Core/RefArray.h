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
#include "Core/InplaceRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Object array reference container.
 * \ingroup Core
 */
template < typename ClassType >
class RefArray
{
	enum
	{
		MaxCapacityAlignment = 256 * 1024,
		MinCapacity = 32
	};

public:
	typedef ClassType* value_type;
	typedef ClassType* pointer;
	typedef ClassType& reference;
	typedef int32_t difference_type;
	typedef size_t size_type;

	/*! Constant iterator. */
	class const_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename RefArray::value_type value_type;
		typedef typename RefArray::difference_type difference_type;
		typedef const value_type pointer;
		typedef const value_type reference;

		const_iterator()
		:	m_item(nullptr)
		{
		}

		const_iterator(const const_iterator& it)
		:	m_item(it.m_item)
		{
		}

		virtual ~const_iterator()
		{
		}

		reference operator * ()
		{
			return *m_item;
		}

		const reference operator * () const
		{
			return *m_item;
		}

		pointer operator -> ()
		{
			return *m_item;
		}

		const pointer operator -> () const
		{
			return *m_item;
		}

		const_iterator operator + (int offset) const
		{
			return const_iterator(m_item + offset);
		}

		const_iterator operator - (int offset) const
		{
			return const_iterator(m_item - offset);
		}

		void operator += (int offset)
		{
			m_item += offset;
		}

		void operator -= (int offset)
		{
			m_item -= offset;
		}

		const_iterator operator ++ ()
		{
			return const_iterator(++m_item);
		}

		const_iterator operator ++ (int)
		{
			return const_iterator(m_item++);
		}

		const_iterator operator -- ()
		{
			return const_iterator(--m_item);
		}

		const_iterator operator -- (int)
		{
			return const_iterator(m_item--);
		}

		bool operator == (const const_iterator& r) const
		{
			return m_item == r.m_item;
		}

		bool operator != (const const_iterator& r) const
		{
			return m_item != r.m_item;
		}

		bool operator < (const const_iterator& r) const
		{
			return m_item < r.m_item;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(m_item - r.m_item);
		}

		const_iterator& operator = (const const_iterator& it)
		{
			m_item = it.m_item;
			return *this;
		}

	protected:
		friend class RefArray;

		value_type* m_item;

		explicit const_iterator(value_type* item)
		:	m_item(item)
		{
		}
	};

	/*! Mutable iterator. */
	class iterator : public const_iterator
	{
	public:
		typedef const_iterator _O;
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename RefArray::value_type value_type;
		typedef typename RefArray::difference_type difference_type;
		typedef value_type pointer;
		typedef value_type reference;

		iterator()
		:	m_owner(nullptr)
		{
		}

		iterator(const iterator& it)
		:	const_iterator(it.m_item)
		,	m_owner(it.m_owner)
		{
		}

		virtual ~iterator()
		{
		}

		InplaceRef< ClassType > operator * ()
		{
			return InplaceRef< ClassType >(*_O::m_item);
		}

		pointer operator -> ()
		{
			return *_O::m_item;
		}

		iterator operator + (int offset) const
		{
			return iterator(_O::m_item + offset, m_owner);
		}

		iterator operator - (int offset) const
		{
			return iterator(_O::m_item - offset, m_owner);
		}

		iterator operator ++ ()
		{
			return iterator(++_O::m_item, m_owner);
		}

		iterator operator ++ (int)
		{
			return iterator(_O::m_item++, m_owner);
		}

		iterator operator -- ()
		{
			return iterator(--_O::m_item, m_owner);
		}

		iterator operator -- (int)
		{
			return iterator(_O::m_item--, m_owner);
		}

		bool operator == (const iterator& r) const
		{
			return _O::m_item == r.m_item;
		}

		bool operator != (const iterator& r) const
		{
			return _O::m_item != r.m_item;
		}

		bool operator < (const iterator& r) const
		{
			return _O::m_item < r.m_item;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(_O::m_item - r.m_item);
		}

		iterator& operator = (const iterator& it)
		{
			_O::m_item = it.m_item;
			return *this;
		}

	protected:
		friend class RefArray;
		void* m_owner;

		explicit iterator(value_type* item, void* owner)
		:	const_iterator(item)
		,	m_owner(owner)
		{
		}
	};

	/*! Construct empty array. */
	RefArray() = default;

	/*! Construct array with size. */
	RefArray(size_type size)
	{
		m_items = new value_type [size];
		m_size = size;
		m_capacity = size;

		for (size_type i = 0; i < m_size; ++i)
			m_items[i] = 0;
	}

	/*! Copy array. */
	RefArray(const RefArray& s)
	{
		m_items = new value_type [s.m_size];
		m_size = s.m_size;
		m_capacity = s.m_size;

		for (size_type i = 0; i < m_size; ++i)
		{
			m_items[i] = s.m_items[i];
			T_SAFE_ADDREF(m_items[i]);
		}
	}

	RefArray(RefArray&& ref) T_NOEXCEPT
	:	m_items(ref.m_items)
	,	m_size(ref.m_size)
	,	m_capacity(ref.m_capacity)
	{
		ref.m_items = nullptr;
		ref.m_size = 0;
		ref.m_capacity = 0;
	}

	virtual ~RefArray()
	{
		clear();
	}

	/*! Clear array.
	 *
	 * All elements are released and
	 * allocated memory are freed.
	 */
	void clear()
	{
		for (size_type i = 0; i < m_size; ++i)
			T_SAFE_RELEASE(m_items[i]);

		delete[] m_items; m_items = nullptr;
		m_size = 0;
		m_capacity = 0;
	}

	/*! Return iterator at beginning of array. */
	iterator begin()
	{
		return iterator(&m_items[0], this);
	}

	/*! Return iterator one past last element. */
	iterator end()
	{
		return iterator(&m_items[m_size], this);
	}

	/*! Return iterator at beginning of array. */
	const_iterator begin() const
	{
		return const_iterator(&m_items[0]);
	}

	/*! Return iterator one past last element. */
	const_iterator end() const
	{
		return const_iterator(&m_items[m_size]);
	}

	/*! Get front element. */
	ClassType* front()
	{
		T_ASSERT(m_items);
		return m_items[0];
	}

	/*! Get back element. */
	ClassType* back()
	{
		T_ASSERT(m_items);
		return m_items[m_size - 1];
	}

	/*! Get front element. */
	const ClassType* front() const
	{
		T_ASSERT(m_items);
		return m_items[0];
	}

	/*! Get back element. */
	const ClassType* back() const
	{
		T_ASSERT(m_items);
		return m_items[m_size - 1];
	}

	/*! Push element as front. */
	void push_front(ClassType* const val)
	{
		T_SAFE_ADDREF(val);
		grow(1);
		for (size_type i = m_size - 1; i > 0; --i)
			m_items[i] = m_items[i - 1];
		m_items[0] = val;
	}

	/*! Pop front element. */
	void pop_front()
	{
		T_SAFE_RELEASE(m_items[0]);
		for (size_type i = 0; i < m_size - 1; ++i)
			m_items[i] = m_items[i + 1];
		shrink(1);
	}

	/*! Push element as back. */
	void push_back(ClassType* const val)
	{
		T_SAFE_ADDREF(val);
		grow(1);
		m_items[m_size - 1] = val;
	}

	/*! Pop back element. */
	void pop_back()
	{
		T_SAFE_RELEASE(m_items[m_size - 1]);
		shrink(1);
	}

	/*! Insert element at specified location. */
	void insert(iterator at, ClassType* const val)
	{
		T_SAFE_ADDREF(val);

		const size_type size = m_size;
		const size_type offset = size_type(at.m_item - m_items);

		grow(1);

		for (size_t i = size; i > offset; --i)
			m_items[i] = m_items[i - 1];

		m_items[offset] = val;
	}

	/*! Insert elements at specified location. */
	iterator insert(iterator at, iterator first, iterator last)
	{
		const size_type size = m_size;
		const size_type offset = size_type(at.m_item - m_items);
		const size_type count = size_type(last.m_item - first.m_item);

		grow(count);

		const int32_t move = int32_t(size - offset);
		for (int32_t i = move - 1; i >= 0; --i)
		{
			T_ASSERT(i + offset < size);
			T_ASSERT(i + offset + count < m_size);
			m_items[i + offset + count] = m_items[i + offset];
		}

		for (size_t i = 0; i < count; ++i)
		{
			T_SAFE_ADDREF(first.m_item[i]);
			m_items[i + offset] = first.m_item[i];
		}

		return iterator(&m_items[offset], this);
	}

	/*! Insert elements at specified location. */
	iterator insert(iterator at, const_iterator first, const_iterator last)
	{
		const size_type size = m_size;
		const size_type offset = size_type(at.m_item - m_items);
		const size_type count = size_type(last.m_item - first.m_item);

		grow(count);

		const int32_t move = int32_t(size - offset);
		for (int32_t i = move - 1; i >= 0; --i)
		{
			T_ASSERT(i + offset < size);
			T_ASSERT(i + offset + count < m_size);
			m_items[i + offset + count] = m_items[i + offset];
		}

		for (size_t i = 0; i < count; ++i)
		{
			T_SAFE_ADDREF(first.m_item[i]);
			m_items[i + offset] = first.m_item[i];
		}

		return iterator(&m_items[offset], this);
	}

	/*! Erase element from array. */
	iterator erase(iterator iter)
	{
		T_ASSERT(m_size > 0);
		T_ASSERT(iter.m_item >= m_items && iter.m_item < &m_items[m_size]);

		T_SAFE_RELEASE(*iter.m_item);

		const size_type offset = size_type(iter.m_item - m_items);
		T_ASSERT(offset < m_size);

		for (size_type i = offset; i < m_size - 1; ++i)
			m_items[i] = m_items[i + 1];

		m_items[m_size - 1] = nullptr;
		shrink(1);

		return iterator(&m_items[offset], this);
	}

	/*! Remove first element from array by value. */
	bool remove(value_type item)
	{
		iterator i = std::find(begin(), end(), item);
		if (i != end())
		{
			erase(i);
			return true;
		}
		else
			return false;
	}

	/*! Erase range of elements from array. */
	void erase(iterator first, iterator last)
	{
		const size_type offset = size_type(first.m_item - m_items);
		const size_type count = size_type(last.m_item - first.m_item);
		const size_type size = m_size - count;

		for (size_type i = offset; i < size; ++i)
		{
			T_SAFE_RELEASE(m_items[i]);
			m_items[i] = m_items[i + count];
			T_SAFE_ADDREF(m_items[i]);
		}

		for (size_type i = size; i < m_size; ++i)
		{
			T_SAFE_RELEASE(m_items[i]);
			m_items[i] = nullptr;
		}

		m_size = size;
	}

	/*! Swap content with another array. */
	void swap(RefArray< ClassType >& src)
	{
		std::swap(m_items, src.m_items);
		std::swap(m_size, src.m_size);
		std::swap(m_capacity, src.m_capacity);
	}

	/*! Check if array is empty. */
	bool empty() const
	{
		return m_size == 0;
	}

	/*! Number of elements in array. */
	size_type size() const
	{
		return m_size;
	}

	/*! Resize array. */
	void resize(size_type size)
	{
		if (size > m_size)
		{
			if (size > m_capacity)
				reserve(size);

			for (size_t i = m_size; i < size; ++i)
				m_items[i] = nullptr;
		}
		else
		{
			for (size_t i = size; i < m_size; ++i)
				T_SAFE_RELEASE(m_items[i]);
		}

		m_size = size;
	}

	/*! Capacity of array.
	 *
	 * Return number of elements which can fit in the
	 * array without allocate more memory.
	 */
	size_type capacity() const
	{
		return m_capacity;
	}

	/*! Reserve capacity of array. */
	void reserve(size_type capacity)
	{
		if (capacity > m_capacity)
		{
			size_t capacityAlignment = (m_capacity > MinCapacity ? m_capacity : MinCapacity);
			if (capacityAlignment > MaxCapacityAlignment)
				capacityAlignment = MaxCapacityAlignment;

			const size_t newCapacity = alignUp(capacity, capacityAlignment);

			value_type* items = new value_type [newCapacity];
			T_FATAL_ASSERT (items);

			for (size_type i = 0; i < m_size; ++i)
				items[i] = m_items[i];
			for (size_type i = m_size; i < newCapacity; ++i)
				items[i] = nullptr;

			delete[] m_items;

			m_items = items;
			m_capacity = newCapacity;
		}
	}

	/*! Return element at specified location. */
	const pointer at(size_type index) const
	{
		return m_items[index];
	}

	/*! Return handle to element at specified location. */
	InplaceRef< ClassType > at(size_type index)
	{
		return InplaceRef< ClassType >(m_items[index]);
	}

	/*! Sort array.
	 *
	 * Prefer use of this method instead
	 * of std::sort directly as this will not cause
	 * reference counts to be modified when
	 * swapping elements thus being faster.
	 *
	 * \param predicate Sort predicate.
	 */
	template < typename PredicateType >
	void sort(PredicateType predicate)
	{
		if (m_size > 1)
			std::sort(&m_items[0], &m_items[m_size], predicate);
	}

	const pointer operator [] (size_type index) const
	{
		return m_items[index];
	}

	InplaceRef< ClassType > operator [] (size_type index)
	{
		return InplaceRef< ClassType >(m_items[index]);
	}

	RefArray< ClassType >& operator = (const RefArray< ClassType >& src)
	{
		for (size_type i = 0; i < m_size; ++i)
		{
			T_SAFE_RELEASE(m_items[i]);
			m_items[i] = nullptr;
		}

		resize(src.m_size);

		for (size_type i = 0; i < m_size; ++i)
		{
			m_items[i] = src.m_items[i];
			T_SAFE_ADDREF(m_items[i]);
		}

		return *this;
	}

	RefArray& operator = (RefArray&& ref) T_NOEXCEPT
	{
		for (size_type i = 0; i < m_size; ++i)
			T_SAFE_RELEASE(m_items[i]);

		delete[] m_items;

		m_items = ref.m_items;
		m_size = ref.m_size;
		m_capacity = ref.m_capacity;

		ref.m_items = nullptr;
		ref.m_size = 0;
		ref.m_capacity = 0;

		return *this;
	}

private:
	value_type* m_items = nullptr;
	size_type m_size = 0;
	size_type m_capacity = 0;

	void grow(size_t count)
	{
		const size_t newSize = m_size + count;
		if (newSize > m_capacity)
			reserve(newSize);
		m_size = newSize;
	}

	void shrink(size_t count)
	{
		m_size -= count;
	}
};

}


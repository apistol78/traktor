#ifndef traktor_RefArray_H
#define traktor_RefArray_H

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

/*! \brief Object array reference container.
 * \ingroup Core
 */
template < typename ClassType >
class RefArray
{
	enum
	{
		ExpandSize = 32
	};

public:
	typedef ClassType* value_type;
	typedef ClassType* pointer;
	typedef ClassType& reference;
	typedef int32_t difference_type;
	typedef size_t size_type;

	/*! \brief Constant iterator. */
	class const_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename RefArray::value_type value_type;
		typedef typename RefArray::difference_type difference_type;
		typedef const value_type pointer;
		typedef const value_type reference;

		const_iterator()
		:	m_item(0)
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

	/*! \brief Mutable iterator. */
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
		:	m_owner(0)
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

	/*! \brief Construct empty array. */
	RefArray()
	:	m_items(0)
	,	m_size(0)
	,	m_capacity(0)
	{
	}

	/*! \brief Construct array with size. */
	RefArray(size_type size)
	:	m_items(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		m_items = new value_type [size];
		m_size = size;
		m_capacity = size;

		for (size_type i = 0; i < m_size; ++i)
			m_items[i] = 0;
	}

	/*! \brief Copy array. */
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

#if defined(T_CXX11)
	RefArray(RefArray&& ref)
	:	m_items(ref.m_items)
	,	m_size(ref.m_size)
	,	m_capacity(ref.m_capacity)
	{
		ref.m_items = 0;
		ref.m_size = 0;
		ref.m_capacity = 0;
	}
#endif

	virtual ~RefArray()
	{
		clear();
	}

	/*! \brief Clear array.
	 *
	 * All elements are released and
	 * allocated memory are freed.
	 */
	void clear()
	{
		for (size_type i = 0; i < m_size; ++i)
			T_SAFE_RELEASE(m_items[i]);

		delete[] m_items; m_items = 0;
		m_size = 0;
		m_capacity = 0;
	}

	/*! \brief Return iterator at beginning of array. */
	iterator begin()
	{
		return iterator(&m_items[0], this);
	}

	/*! \brief Return iterator one past last element. */
	iterator end()
	{
		return iterator(&m_items[m_size], this);
	}

	/*! \brief Return iterator at beginning of array. */
	const_iterator begin() const
	{
		return const_iterator(&m_items[0]);
	}

	/*! \brief Return iterator one past last element. */
	const_iterator end() const
	{
		return const_iterator(&m_items[m_size]);
	}

	/*! \brief Get front element. */
	ClassType* front()
	{
		T_ASSERT (m_items);
		return m_items[0];
	}

	/*! \brief Get back element. */
	ClassType* back()
	{
		T_ASSERT (m_items);
		return m_items[m_size - 1];
	}

	/*! \brief Get front element. */
	const ClassType* front() const
	{
		T_ASSERT (m_items);
		return m_items[0];
	}

	/*! \brief Get back element. */
	const ClassType* back() const
	{
		T_ASSERT (m_items);
		return m_items[m_size - 1];
	}

	/*! \brief Push element as front. */
	void push_front(ClassType* const val)
	{
		T_SAFE_ADDREF(val);
		grow(1);
		for (size_type i = m_size - 1; i > 0; --i)
			m_items[i] = m_items[i - 1];
		m_items[0] = val;
	}

	/*! \brief Pop front element. */
	void pop_front()
	{
		T_SAFE_RELEASE(m_items[0]);
		for (size_type i = 0; i < m_size - 1; ++i)
			m_items[i] = m_items[i + 1];
		shrink(1);
	}

	/*! \brief Push element as back. */
	void push_back(ClassType* const val)
	{
		T_SAFE_ADDREF(val);
		grow(1);
		m_items[m_size - 1] = val;
	}

	/*! \brief Pop back element. */
	void pop_back()
	{
		T_SAFE_RELEASE(m_items[m_size - 1]);
		shrink(1);
	}

	/*! \brief Insert element at specified location. */
	void insert(iterator at, ClassType* const val)
	{
		T_SAFE_ADDREF(val);

		size_type size = m_size;
		size_type offset = size_type(at.m_item - m_items);

		grow(1);

		for (size_t i = size; i > offset; --i)
			m_items[i] = m_items[i - 1];

		m_items[offset] = val;
	}

	/*! \brief Insert elements at specified location. */
	iterator insert(iterator at, iterator first, iterator last)
	{
		size_type size = m_size;
		size_type offset = size_type(at.m_item - m_items);
		size_type count = size_type(last.m_item - first.m_item);

		grow(count);

		size_t move = std::min< size_t >(size, count);
		for (size_t i = offset; i < offset + move; ++i)
			m_items[i + count] = m_items[i];

		for (size_t i = 0; i < count; ++i)
		{
			T_SAFE_ADDREF(first.m_item[i]);
			m_items[i + offset] = first.m_item[i];
		}

		return iterator(&m_items[offset], this);
	}

	/*! \brief Insert elements at specified location. */
	iterator insert(iterator at, const_iterator first, const_iterator last)
	{
		size_type size = m_size;
		size_type offset = size_type(at.m_item - m_items);
		size_type count = size_type(last.m_item - first.m_item);

		grow(count);

		int32_t move = int32_t(size - offset);
		for (int32_t i = move - 1; i >= 0; --i)
		{
			T_ASSERT (i + offset < size);
			T_ASSERT (i + offset + count < m_size);
			m_items[i + offset + count] = m_items[i + offset];
		}

		for (size_t i = 0; i < count; ++i)
		{
			T_SAFE_ADDREF(first.m_item[i]);
			m_items[i + offset] = first.m_item[i];
		}

		return iterator(&m_items[offset], this);
	}

	/*! \brief Erase element from array. */
	iterator erase(iterator iter)
	{
		T_ASSERT (m_size > 0);
		T_ASSERT (iter.m_item >= m_items && iter.m_item < &m_items[m_size]);

		T_SAFE_RELEASE(*iter.m_item);

		size_type offset = size_type(iter.m_item - m_items);
		T_ASSERT (offset < m_size);

		for (size_type i = offset; i < m_size - 1; ++i)
			m_items[i] = m_items[i + 1];

		m_items[m_size - 1] = 0;
		shrink(1);

		return iterator(&m_items[offset], this);
	}

	/*! \brief Remove first element from array by value. */
	void remove(value_type item)
	{
		iterator i = std::find(begin(), end(), item);
		if (i != end())
			erase(i);
	}

	/*! \brief Erase range of elements from array. */
	void erase(iterator first, iterator last)
	{
		size_type offset = size_type(first.m_item - m_items);
		size_type count = size_type(last.m_item - first.m_item);
		size_type size = m_size - count;

		for (size_type i = offset; i < size; ++i)
		{
			T_SAFE_RELEASE(m_items[i]);
			m_items[i] = m_items[i + count];
			T_SAFE_ADDREF(m_items[i]);
		}

		for (size_type i = size; i < m_size; ++i)
		{
			T_SAFE_RELEASE(m_items[i]);
			m_items[i] = 0;
		}

		m_size = size;
	}

	/*! \brief Swap content with another array. */
	void swap(RefArray< ClassType >& src)
	{
		std::swap(m_items, src.m_items);
		std::swap(m_size, src.m_size);
		std::swap(m_capacity, src.m_capacity);
	}

	/*! \brief Check if array is empty. */
	bool empty() const
	{
		return m_size == 0;
	}

	/*! \brief Number of elements in array. */
	size_type size() const
	{
		return m_size;
	}

	/*! \brief Resize array. */
	void resize(size_type size)
	{
		if (size > m_size)
		{
			if (size > m_capacity)
			{
				size_t capacity = (size & ~(ExpandSize - 1)) + ExpandSize;
				reserve(capacity);
			}

			for (size_t i = m_size; i < size; ++i)
				m_items[i] = 0;
		}
		else
		{
			for (size_t i = size; i < m_size; ++i)
				T_SAFE_RELEASE(m_items[i]);
		}

		m_size = size;
	}

	/*! \brief Capacity of array.
	 *
	 * Return number of elements which can fit in the
	 * array without allocate more memory.
	 */
	size_type capacity() const
	{
		return m_capacity;
	}

	/*! \brief Reserve capacity of array. */
	void reserve(size_type size)
	{
		if (size > m_capacity)
		{
			value_type* items = new value_type [size];
			T_FATAL_ASSERT (items);

			for (size_type i = 0; i < m_size; ++i)
				items[i] = m_items[i];
			for (size_type i = m_size; i < size; ++i)
				items[i] = 0;

			delete[] m_items;

			m_items = items;
			m_capacity = size;
		}
	}

	/*! \brief Return element at specified location. */
	const pointer at(size_type index) const
	{
		return m_items[index];
	}

	/*! \brief Return handle to element at specified location. */
	InplaceRef< ClassType > at(size_type index)
	{
		return InplaceRef< ClassType >(m_items[index]);
	}

	/*! \brief Sort array.
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
			m_items[i] = 0;
		}

		resize(src.m_size);
		
		for (size_type i = 0; i < m_size; ++i)
		{
			m_items[i] = src.m_items[i];
			T_SAFE_ADDREF(m_items[i]);
		}

		return *this;
	}

#if defined(T_CXX11)
	RefArray& operator = (RefArray&& ref)
	{
		for (size_type i = 0; i < m_size; ++i)
			T_SAFE_RELEASE(m_items[i]);

		delete[] m_items;

		m_items = ref.m_items;
		m_size = ref.m_size;
		m_capacity = ref.m_capacity;

		ref.m_items = 0;
		ref.m_size = 0;
		ref.m_capacity = 0;

		return *this;
	}
#endif

private:
	value_type* m_items;
	size_type m_size;
	size_type m_capacity;

	void grow(size_t count)
	{
		size_t newSize = m_size + count;
		if (newSize > m_capacity)
		{
			size_t capacity = (newSize & ~(ExpandSize - 1)) + ExpandSize;
			reserve(capacity);
		}
		m_size = newSize;
	}

	void shrink(size_t count)
	{
		m_size -= count;
	}
};

}

#endif	// traktor_RefArray_H

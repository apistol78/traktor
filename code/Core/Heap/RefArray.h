#ifndef traktor_RefArray_H
#define traktor_RefArray_H

#include <algorithm>
#include "Core/Heap/InplaceRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Object array reference container.
 * \ingroup Core
 */
template < typename Class >
class RefArray : public RefBase
{
	enum
	{
		ExpandSize = 32
	};

	struct ScopeLockRef
	{
		ScopeLockRef()
		{
			Heap::lockRef();
		}

		~ScopeLockRef()
		{
			Heap::unlockRef();
		}
	};

public:
	typedef Class* value_type;
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

		reference operator * () const
		{
			return *m_item;
		}

		pointer operator -> () const
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
		{
			Heap::lockRef();
		}

		iterator(const iterator& it)
		:	const_iterator(it.m_item)
		{
			Heap::lockRef();
		}

		virtual ~iterator()
		{
			Heap::unlockRef();
		}

		InplaceRef< Class > operator * ()
		{
			return *_O::m_item;
		}

		pointer operator -> ()
		{
			return *_O::m_item;
		}

		iterator operator + (int offset) const
		{
			return iterator(_O::m_item + offset);
		}

		iterator operator - (int offset) const
		{
			return iterator(_O::m_item - offset);
		}

		iterator operator ++ ()
		{
			return iterator(++_O::m_item);
		}

		iterator operator ++ (int)
		{
			return iterator(_O::m_item++);
		}

		iterator operator -- ()
		{
			return iterator(--_O::m_item);
		}

		iterator operator -- (int)
		{
			return iterator(_O::m_item--);
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

		explicit iterator(value_type* item)
		:	const_iterator(item)
		{
			Heap::lockRef();
		}
	};

	RefArray()
	:	m_items(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		Heap::registerRef(this, 0);
	}

	RefArray(size_type size)
	:	m_items(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		Heap::registerRef(this, 0);

		m_items = new value_type [size];
		m_size = size;
		m_capacity = size;

		for (size_type i = 0; i < m_size; ++i)
			m_items[i] = 0;
	}

	RefArray(const RefArray< Class >& s)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		Heap::registerRef(this, 0);

		m_items = new value_type [s.m_size];
		m_size = s.m_size;
		m_capacity = s.m_size;

		for (size_type i = 0; i < m_size; ++i)
		{
			m_items[i] = s.m_items[i];
			Heap::incrementRef(m_items[i]);
		}
	}

	virtual ~RefArray()
	{
		Heap::unregisterRef(this, 0);
		clear();
	}

	void clear()
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		for (size_type i = 0; i < m_size; ++i)
			Heap::decrementRef(m_items[i]);

		delete[] m_items, m_items = 0;
		m_size = 0;
		m_capacity = 0;
	}

	iterator begin()
	{
		return iterator(&m_items[0]);
	}

	iterator end()
	{
		return iterator(&m_items[m_size]);
	}

	const_iterator begin() const
	{
		return const_iterator(&m_items[0]);
	}

	const_iterator end() const
	{
		return const_iterator(&m_items[m_size]);
	}

	Class* front()
	{
		T_ASSERT (m_items);
		return m_items[0];
	}

	Class* back()
	{
		T_ASSERT (m_items);
		return m_items[m_size - 1];
	}

	const Class* front() const
	{
		T_ASSERT (m_items);
		return m_items[0];
	}

	const Class* back() const
	{
		T_ASSERT (m_items);
		return m_items[m_size - 1];
	}

	void push_front(Class* const val)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);
		Heap::incrementRef(val);
		grow(1);
		for (size_type i = m_size - 1; i > 0; --i)
			m_items[i] = m_items[i - 1];
		m_items[0] = val;
	}

	void pop_front()
	{
		T_ANONYMOUS_VAR(ScopeLockRef);
		Heap::decrementRef(m_items[0]);
		for (size_type i = 0; i < m_size - 1; ++i)
			m_items[i] = m_items[i + 1];
		shrink(1);
	}

	void push_back(Class* const val)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);
		Heap::incrementRef(val);
		grow(1);
		m_items[m_size - 1] = val;
	}

	void pop_back()
	{
		T_ANONYMOUS_VAR(ScopeLockRef);
		Heap::decrementRef(m_items[m_size - 1]);
		shrink(1);
	}

	void insert(iterator at, Class* const val)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		Heap::incrementRef(val);

		size_type size = m_size;
		size_type offset = size_type(at.m_item - m_items);

		grow(1);

		for (size_t i = size; i > offset; --i)
			m_items[i] = m_items[i - 1];

		m_items[offset] = val;
	}

	void insert(iterator at, iterator first, iterator last)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		size_type size = m_size;
		size_type offset = size_type(at.m_item - m_items);
		size_type count = size_type(last.m_item - first.m_item);

		grow(count);

		size_t move = std::min< size_t >(size, count);
		for (size_t i = offset; i < offset + move; ++i)
			m_items[i + count] = m_items[i];

		for (size_t i = 0; i < count; ++i)
		{
			Heap::incrementRef(first.m_item[i]);
			m_items[i + offset] = first.m_item[i];
		}

		return iterator(&m_items[offset]);
	}

	iterator erase(iterator iter)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		Heap::decrementRef(iter.m_item);

		size_type offset = size_type(iter.m_item - m_items);
		for (size_type i = offset; i < m_size; ++i)
			m_items[i] = m_items[i + 1];

		m_items[m_size - 1] = 0;
		shrink(1);

		return iterator(&m_items[offset]);
	}

	void remove(value_type item)
	{
		iterator i = std::find(begin(), end(), item);
		if (i != end())
			erase(i);
	}

	void erase(iterator first, iterator last)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		size_type offset = size_type(first.m_item - m_items);
		size_type count = size_type(last.m_item - first.m_item);
		size_type size = m_size - count;

		for (size_type i = offset; i < size; ++i)
		{
			Heap::decrementRef(m_items[i]);
			m_items[i] = m_items[i + count];
		}

		for (size_type i = size; i < m_size; ++i)
			m_items[i] = 0;

		m_size = size;
	}

	void swap(RefArray< Class >& src)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);
		std::swap(m_items, src.m_items);
		std::swap(m_size, src.m_size);
		std::swap(m_capacity, src.m_capacity);
	}

	bool empty() const
	{
		return m_size == 0;
	}

	size_type size() const
	{
		return m_size;
	}

	void resize(size_type size)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

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
				Heap::decrementRef(m_items[i]);
		}

		m_size = size;
	}

	size_type capacity() const
	{
		return m_capacity;
	}

	void reserve(size_type size)
	{
		if (size > m_capacity)
		{
			T_ANONYMOUS_VAR(ScopeLockRef);

			value_type* items = new value_type [size];
			T_FATAL_ASSERT (items);

			for (size_type i = 0; i < m_size; ++i)
				items[i] = m_items[i];
			for (size_type i = m_size; i < size; ++i)
				items[i] = 0;

			m_items = items;
			m_capacity = size;
		}
	}

	InplaceRef< Class > at(size_type index)
	{
		return m_items[index];
	}

	Class* at(size_type index) const
	{
		return m_items[index];
	}

	InplaceRef< Class > operator [] (size_type index)
	{
		return m_items[index];
	}

	Class* operator [] (size_type index) const
	{
		return m_items[index];
	}

	RefArray< Class >& operator = (const RefArray< Class >& src)
	{
		T_ANONYMOUS_VAR(ScopeLockRef);

		for (size_type i = 0; i < m_size; ++i)
		{
			Heap::decrementRef(m_items[i]);
			m_items[i] = 0;
		}

		resize(src.m_size);
		
		for (size_type i = 0; i < m_size; ++i)
		{
			m_items[i] = src.m_items[i];
			Heap::incrementRef(m_items[i]);
		}

		return *this;
	}

protected:
	virtual void visit(IVisitor& visitor)
	{
		for (size_type i = 0; i < m_size; ++i)
			visitor((void*)m_items[i]);
	}

	virtual void invalidate(void* object)
	{
		for (size_type i = 0; i < m_size; ++i)
		{
			if (m_items[i] == object)
				m_items[i] = 0;
		}
	}

private:
	value_type* m_items;
	size_type m_size;
	size_type m_capacity;

	void grow(size_t count)
	{
		size_t newSize = m_size + count;
		if (newSize >= m_capacity)
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

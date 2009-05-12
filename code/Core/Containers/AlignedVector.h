#ifndef traktor_AlignedVector_H
#define traktor_AlignedVector_H

#include <iterator>
#include "Core/Config.h"
#include "Core/Heap/Alloc.h"

namespace traktor
{

/*! \brief Construct/destruct items policy.
 *
 * Default policy when constructing or destroying
 * items in the AlignedVector container.
 */
template < typename ItemType >
struct AlignedVectorConstructor
{
	static void construct(ItemType& uninitialized, const ItemType& source)
	{
		new (&uninitialized) ItemType(source);
	}

	static void destroy(ItemType& item)
	{
		item.~ItemType();
	}
};

/*! \brief Vector container which strict item alignment.
 * \ingroup Core
 *
 * This container is designed to be a drop-in replacement
 * for STL vector.
 */
template < typename ItemType, typename Constructor = AlignedVectorConstructor< ItemType > >
class AlignedVector
{
	enum
	{
		ExpandSize = 64,
		Alignment = 16
	};

public:
	typedef ItemType value_type;
	typedef typename int difference_type;
	typedef typename const ItemType* const_pointer;
	typedef typename const ItemType& const_reference;
	typedef typename ItemType* pointer;
	typedef typename ItemType& reference;

	class const_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef typename int difference_type;
		typedef typename const ItemType* pointer;
		typedef typename const ItemType& reference;

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

		const_iterator operator - (int offset) const
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

		const_iterator& operator -- ()
		{
			--m_ptr;
			return *this;
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

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(m_ptr - r.m_ptr);
		}

	protected:
		friend class AlignedVector;
		ItemType* m_ptr;

		explicit const_iterator(ItemType* ptr)
		:	m_ptr(ptr)
		{
		}
	};

	class iterator : public const_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef typename int difference_type;
		typedef typename ItemType* pointer;
		typedef typename ItemType& reference;

		reference operator * ()
		{
			return *m_ptr;
		}

		pointer operator -> ()
		{
			return m_ptr;
		}

		iterator operator + (int offset) const
		{
			return iterator(m_ptr + offset);
		}

		iterator operator - (int offset) const
		{
			return iterator(m_ptr - offset);
		}

		iterator operator ++ ()		// pre-fix
		{
			return iterator(++m_ptr);
		}

		iterator operator ++ (int)	// post-fix
		{
			return iterator(m_ptr++);
		}

		iterator& operator -- ()
		{
			--m_ptr;
			return *this;
		}

		bool operator == (const iterator& r) const
		{
			return m_ptr == r.m_ptr;
		}

		bool operator != (const iterator& r) const
		{
			return m_ptr != r.m_ptr;
		}

		bool operator < (const iterator& r) const
		{
			return m_ptr < r.m_ptr;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(m_ptr - r.m_ptr);
		}

	protected:
		friend class AlignedVector;

		explicit iterator(ItemType* ptr)
		:	const_iterator(ptr)
		{
		}
	};

	AlignedVector()
	:	m_data(0)
	,	m_size(0)
	,	m_capacity(0)
	{
	}

	AlignedVector(size_t size)
	:	m_data(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		resize(size);
	}

	AlignedVector(const AlignedVector< ItemType >& src)
	:	m_data(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		insert(begin(), src.begin(), src.end());
	}

	virtual ~AlignedVector()
	{
		clear();
	}

	size_t size() const
	{
		return m_size;
	}

	size_t capacity() const
	{
		return m_capacity;
	}

	bool empty() const
	{
		return m_size == 0;
	}

	void clear()
	{
		for (size_t i = 0; i < m_size; ++i)
			Constructor::destroy(m_data[i]);

		freeAlign(m_data);

		m_data = 0;
		m_size = 0;
		m_capacity = 0;
	}

	void resize(size_t size)
	{
		resize(size, ItemType());
	}

	void resize(size_t size, const ItemType& pad)
	{
		if (size > m_size)
		{
			if (size > m_capacity)
			{
				size_t capacity = (size & ~(ExpandSize - 1)) + ExpandSize;
				reserve(capacity);
			}

			for (size_t i = m_size; i < size; ++i)
				Constructor::construct(m_data[i], pad);
		}
		else
		{
			for (size_t i = size; i < m_size; ++i)
				Constructor::destroy(m_data[i]);
		}

		m_size = size;
	}

	void reserve(size_t capacity)
	{
		if (capacity > m_capacity)
		{
			ItemType* data = reinterpret_cast< ItemType* >(allocAlign(capacity * sizeof(ItemType), Alignment));

			if (m_data)
			{
				for (size_t i = 0; i < m_size; ++i)
				{
					Constructor::construct(data[i], m_data[i]);
					Constructor::destroy(m_data[i]);
				}
				freeAlign(m_data);
			}

			m_data = data;
			m_capacity = capacity;
		}
	}

	void push_back(const ItemType& item)
	{
		grow(1);
		Constructor::construct(m_data[m_size - 1], item);
	}

	void pop_back()
	{
		T_ASSERT (m_size > 0);
		Constructor::destroy(m_data[m_size - 1]);
		shrink(1);
	}

	ItemType& front()
	{
		T_ASSERT (m_size > 0);
		return *m_data;
	}

	const ItemType& front() const
	{
		T_ASSERT (m_size > 0);
		return *m_data;
	}

	ItemType& back()
	{
		T_ASSERT (m_size > 0);
		return m_data[m_size - 1];
	}

	const ItemType& back() const
	{
		T_ASSERT (m_size > 0);
		return m_data[m_size - 1];
	}

	iterator begin()
	{
		return iterator(m_data);
	}

	iterator end()
	{
		return iterator(&m_data[m_size]);
	}

	const_iterator begin() const
	{
		return const_iterator(m_data);
	}

	const_iterator end() const
	{
		return const_iterator(&m_data[m_size]);
	}

	iterator erase(const iterator& where)
	{
		size_t offset = size_t(where.m_ptr - m_data);

		for (size_t i = offset; i < m_size; ++i)
			m_data[i] = m_data[i + 1];

		Constructor::destroy(m_data[m_size - 1]);
		shrink(1);

		return iterator(&m_data[offset]);
	}

	iterator insert(const iterator& where, const ItemType& item)
	{
		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);

		grow(1);

		// Initialize grown item.
		Constructor::construct(m_data[size], ItemType());

		// Move items to make room for item to be inserted.
		for (size_t i = size; i > offset; --i)
			m_data[i] = m_data[i - 1];

		// Copy insert item into location.
		Constructor::destroy(m_data[offset]);
		Constructor::construct(m_data[offset], item);

		return iterator(&m_data[offset]);
	}

	iterator insert(const iterator& where, const const_iterator& from, const const_iterator& to)
	{
		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);
		size_t count = size_t(to.m_ptr - from.m_ptr);

		grow(count);

		// Initialize grown items.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_data[i + size], ItemType());

		// Move items to make room for items to be inserted.
		size_t move = min(size, count);
		for (size_t i = offset; i < offset + move; ++i)
			m_data[i + count] = m_data[i];

		// Copy insert items into location.
		for (size_t i = 0; i < count; ++i)
		{
			Constructor::destroy(m_data[i + offset]);
			Constructor::construct(m_data[i + offset], from.m_ptr[i]);
		}

		return iterator(&m_data[offset]);
	}

	ItemType& operator [] (size_t index)
	{
		T_ASSERT (index < m_size);
		return m_data[index];
	}

	const ItemType& operator [] (size_t index) const
	{
		T_ASSERT (index < m_size);
		return m_data[index];
	}

	AlignedVector< ItemType >& operator = (const AlignedVector< ItemType >& src)
	{
		clear();
		insert(begin(), src.begin(), src.end());
		return *this;
	}

private:
	ItemType* m_data;
	size_t m_size;
	size_t m_capacity;

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

#endif	// traktor_AlignedVector_H

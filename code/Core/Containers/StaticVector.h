#ifndef traktor_StaticVector_H
#define traktor_StaticVector_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Static vector container.
 * \ingroup Core
 */
template < typename ItemType, size_t Capacity >
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

		const_iterator()
		:	m_ptr(0)
		{
		}

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
		ItemType* m_ptr;

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

		reference operator * ()
		{
			return *_O::m_ptr;
		}

		pointer operator -> ()
		{
			return _O::m_ptr;
		}

		iterator operator + (int offset) const
		{
			return iterator(_O::m_ptr + offset);
		}

		iterator operator - (int offset) const
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

	StaticVector()
	:	m_size(0)
	{
	}

	StaticVector(size_t size)
	:	m_size(size)
	{
	}

	StaticVector(size_t size, const ItemType& value)
	:	m_size(size)
	{
		for (uint32_t i = 0; i < m_size; ++i)
			m_items[i] = value;
	}

	/*! \brief Get number of elements in vector.
	 *
	 * \return Number of elements.
	 */
	size_t size() const
	{
		return m_size;
	}

	/*! \brief Get number of elements allocated by vector.
	 *
	 * \return Number of allocated elements.
	 */
	size_t capacity() const
	{
		return Capacity;
	}

	/*! \brief Check if vector is empty.
	 *
	 * \return True if vector empty.
	 */
	bool empty() const
	{
		return m_size == 0;
	}

	/*! \brief Clear vector. */
	void clear()
	{
		m_size = 0;
	}

	/*! \brief Assign value to vector.
	 *
	 * \param size Size of vector.
	 * \param value Value of each element.
	 */
	void assign(uint32_t size, const ItemType& value)
	{
		T_ASSERT (size < Capacity);
		m_size = size;
		for (uint32_t i = 0; i < m_size; ++i)
			m_items[i] = value;
	}

	/*! \brief Push value onto vector.
	 *
	 * \return Item value.
	 */
	ItemType& push_back()
	{
		T_ASSERT (m_size < Capacity);
		return m_items[m_size++];
	}

	/*! \brief Push value onto vector.
	 *
	 * \param item Item value.
	 */
	void push_back(const ItemType& item)
	{
		T_ASSERT (m_size < Capacity);
		m_items[m_size++] = item;
	}

	/*! \brief Pop value from vector. */
	void pop_back()
	{
		T_ASSERT (m_size > 0);
		--m_size;
	}

	/*! \brief Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& front()
	{
		T_ASSERT (m_size > 0);
		return m_items[0];
	}

	/*! \brief Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& front() const
	{
		T_ASSERT (m_size > 0);
		return m_items[0];
	}

	/*! \brief Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& back()
	{
		T_ASSERT (m_size > 0);
		return m_items[m_size - 1];
	}

	/*! \brief Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& back() const
	{
		T_ASSERT (m_size > 0);
		return m_items[m_size - 1];
	}

	/*! \brief Return iterator at first element.
	 *
	 * \return Iterator.
	 */
	iterator begin()
	{
		return iterator(m_items);
	}

	/*! \brief Return iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	iterator end()
	{
		return iterator(&m_items[m_size]);
	}

	/*! \brief Return constant iterator at first element.
	 *
	 * \return Iterator.
	 */
	const_iterator begin() const
	{
		return const_iterator(m_items);
	}

	/*! \brief Return constant iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	const_iterator end() const
	{
		return const_iterator(&m_items[m_size]);
	}

	/*! \brief Erase element.
	 *
	 * \param where Iterator at element.
	 * \return New iterator at next element.
	 */
	iterator erase(const iterator& where)
	{
		size_t offset = size_t(where.m_ptr - m_items);

		for (size_t i = offset; i < m_size - 1; ++i)
			m_items[i] = m_items[i + 1];

		--m_size;
		return iterator(&m_items[offset]);
	}

	operator ItemType* ()
	{
		return m_items;
	}

	operator const ItemType* () const
	{
		return m_items;
	}

private:
	ItemType m_items[Capacity];
	size_t m_size;
};

}

#endif	// traktor_StaticVector_H

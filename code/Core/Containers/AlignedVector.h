#pragma once

#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>
#include "Core/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/Align.h"

namespace traktor
{

/*! Construct/destruct items policy.
 * \ingroup Core
 *
 * Default policy when constructing or destroying
 * items in the AlignedVector container.
 */
template < typename ItemType, bool pod >
struct AlignedVectorConstructorBase
{
};

template < typename ItemType >
struct AlignedVectorConstructorBase < ItemType, false >
{
	static void construct(ItemType& uninitialized)
	{
		new (&uninitialized) ItemType();
	}

	static void construct(ItemType& uninitialized, const ItemType& source)
	{
		new (&uninitialized) ItemType(source);
	}

	static void destroy(ItemType& item)
	{
		item.~ItemType();
	}

	static void move(ItemType& uninitialized, ItemType& source)
	{
		new (&uninitialized) ItemType(std::move(source));
	}
};

template < typename ItemType >
struct AlignedVectorConstructorBase < ItemType, true >
{
	static void construct(ItemType& uninitialized)
	{
	}

	static void construct(ItemType& uninitialized, const ItemType& source)
	{
		uninitialized = source;
	}

	static void destroy(ItemType& item)
	{
	}

	static void move(ItemType& uninitialized, ItemType& source)
	{
		uninitialized = std::move(source);
	}
};

template < typename ItemType >
struct AlignedVectorConstructor : public AlignedVectorConstructorBase< ItemType, std::is_pod< ItemType >::value >
{
};

/*! Vector container which strict item alignment.
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
		MaxCapacityAlignment = 256 * 1024,
		MinCapacity = 32,
		Alignment = 16
	};

public:
	typedef ItemType value_type;
	typedef size_t size_type;
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

		reference operator [] (int32_t offset) const
		{
			return *(m_ptr + offset);
		}

		const_iterator operator + (int32_t offset) const
		{
			return const_iterator(m_ptr + offset);
		}

		const_iterator operator + (uint32_t offset) const
		{
			return const_iterator(m_ptr + offset);
		}

#if !defined(__ANDROID__) && !defined(__RPI__)
		const_iterator operator + (size_t offset) const
		{
			return const_iterator(m_ptr + offset);
		}
#endif

		const_iterator operator - (int32_t offset) const
		{
			return const_iterator(m_ptr - offset);
		}

		const_iterator operator - (uint32_t offset) const
		{
			return const_iterator(m_ptr - offset);
		}

#if !defined(__ANDROID__) && !defined(__RPI__)
		const_iterator operator - (size_t offset) const
		{
			return const_iterator(m_ptr - offset);
		}
#endif

		void operator += (int32_t offset)
		{
			m_ptr += offset;
		}

		void operator -= (int32_t offset)
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
		friend class AlignedVector;
		ItemType* m_ptr = nullptr;

		explicit const_iterator(ItemType* ptr)
		:	m_ptr(ptr)
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

		iterator() = default;

		reference operator * () const
		{
			return *_O::m_ptr;
		}

		pointer operator -> () const
		{
			return _O::m_ptr;
		}

		reference operator [] (int32_t offset) const
		{
			return *(_O::m_ptr + offset);
		}

		iterator operator + (int32_t offset) const
		{
			return iterator(_O::m_ptr + offset);
		}

		iterator operator + (uint32_t offset) const
		{
			return iterator(_O::m_ptr + offset);
		}		

#if !defined(__ANDROID__) && !defined(__RPI__)
		iterator operator + (size_t offset) const
		{
			return iterator(_O::m_ptr + offset);
		}
#endif

		iterator operator - (int32_t offset) const
		{
			return iterator(_O::m_ptr - offset);
		}

		iterator operator - (uint32_t offset) const
		{
			return iterator(_O::m_ptr - offset);
		}

#if !defined(__ANDROID__) && !defined(__RPI__)
		iterator operator - (size_t offset) const
		{
			return iterator(_O::m_ptr - offset);
		}
#endif

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
		friend class AlignedVector;

		explicit iterator(ItemType* ptr)
		:	const_iterator(ptr)
		{
		}
	};

	class const_reverse_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef const value_type* pointer;
		typedef const value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		const_reverse_iterator() = default;

		const_reverse_iterator(const const_reverse_iterator& r)
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

		const_reverse_iterator operator + (int32_t offset) const
		{
			return const_reverse_iterator(m_ptr - offset);
		}

		const_reverse_iterator operator + (size_t offset) const
		{
			return const_reverse_iterator(m_ptr - offset);
		}

		const_reverse_iterator operator - (int32_t offset) const
		{
			return const_reverse_iterator(m_ptr + offset);
		}

		const_reverse_iterator operator - (size_t offset) const
		{
			return const_reverse_iterator(m_ptr + offset);
		}

		void operator += (int offset)
		{
			m_ptr -= offset;
		}

		void operator -= (int offset)
		{
			m_ptr += offset;
		}

		const_reverse_iterator operator ++ ()		// pre-fix
		{
			return const_reverse_iterator(--m_ptr);
		}

		const_reverse_iterator operator ++ (int)	// post-fix
		{
			return const_reverse_iterator(m_ptr--);
		}

		const_reverse_iterator operator -- ()
		{
			return const_reverse_iterator(++m_ptr);
		}

		const_reverse_iterator operator -- (int)
		{
			return const_reverse_iterator(m_ptr++);
		}

		bool operator == (const const_reverse_iterator& r) const
		{
			return m_ptr == r.m_ptr;
		}

		bool operator != (const const_reverse_iterator& r) const
		{
			return m_ptr != r.m_ptr;
		}

		difference_type operator - (const const_reverse_iterator& r) const
		{
			return difference_type(m_ptr - r.m_ptr);
		}

		const_reverse_iterator& operator = (const const_reverse_iterator& r)
		{
			m_ptr = r.m_ptr;
			return *this;
		}

		const_pointer _const_ptr() const
		{
			return m_ptr;
		}

	protected:
		friend class AlignedVector;
		ItemType* m_ptr = nullptr;

		explicit const_reverse_iterator(ItemType* ptr)
		:	m_ptr(ptr)
		{
		}
	};

	class reverse_iterator : public const_reverse_iterator
	{
	public:
		typedef const_reverse_iterator _O;
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

		reverse_iterator operator + (int32_t offset) const
		{
			return reverse_iterator(_O::m_ptr - offset);
		}

		reverse_iterator operator + (size_t offset) const
		{
			return reverse_iterator(_O::m_ptr - offset);
		}

		reverse_iterator operator - (int32_t offset) const
		{
			return reverse_iterator(_O::m_ptr + offset);
		}

		reverse_iterator operator - (size_t offset) const
		{
			return reverse_iterator(_O::m_ptr + offset);
		}

		reverse_iterator operator ++ ()		// pre-fix
		{
			return reverse_iterator(--_O::m_ptr);
		}

		reverse_iterator operator ++ (int)	// post-fix
		{
			return reverse_iterator(_O::m_ptr--);
		}

		reverse_iterator operator -- ()
		{
			return reverse_iterator(++_O::m_ptr);
		}

		reverse_iterator operator -- (int)
		{
			return reverse_iterator(_O::m_ptr++);
		}

		bool operator == (const reverse_iterator& r) const
		{
			return _O::m_ptr == r.m_ptr;
		}

		bool operator != (const reverse_iterator& r) const
		{
			return _O::m_ptr != r.m_ptr;
		}

		difference_type operator - (const const_reverse_iterator& r) const
		{
			return difference_type(_O::m_ptr - r._const_ptr());
		}

	protected:
		friend class AlignedVector;

		explicit reverse_iterator(ItemType* ptr)
		:	const_reverse_iterator(ptr)
		{
		}
	};

	AlignedVector()
	:	m_data(nullptr)
	,	m_size(0)
	,	m_capacity(0)
	{
	}

	AlignedVector(const AlignedVector< ItemType >& src)
	:	m_data(nullptr)
	,	m_size(0)
	,	m_capacity(0)
	{
		insert(begin(), src.begin(), src.end());
	}

	AlignedVector(AlignedVector< ItemType >&& src) noexcept
	:	m_data(src.m_data)
	,	m_size(src.m_size)
	,	m_capacity(src.m_capacity)
	{
		src.m_data = nullptr;
		src.m_size = 0;
		src.m_capacity = 0;
	}

	AlignedVector(const std::initializer_list< ItemType >& iv) noexcept
	:	m_data(nullptr)
	,	m_size(0)
	,	m_capacity(0)
	{
		reserve(iv.size());
		for (auto it = std::begin(iv); it != std::end(iv); ++it)
			push_back(*it);
	}

	explicit AlignedVector(size_t size, const ItemType& value = ItemType())
	:	m_data(nullptr)
	,	m_size(0)
	,	m_capacity(0)
	{
		resize(size, value);
	}

	template < typename IteratorType >
	explicit AlignedVector(const IteratorType& from, const IteratorType& to)
	:	m_data(nullptr)
	,	m_size(0)
	,	m_capacity(0)
	{
		reserve((size_t)(to - from));
		for (IteratorType i = from; i != to; ++i)
			push_back(*i);
	}

	virtual ~AlignedVector()
	{
		clear();
	}

	/*! Get number of elements in vector.
	 *
	 * \return Number of elements.
	 */
	size_t size() const
	{
		return m_size;
	}

	/*! Get number of elements allocated by vector.
	 *
	 * \return Number of allocated elements.
	 */
	size_t capacity() const
	{
		return m_capacity;
	}

	/*! Check if vector is empty.
	 *
	 * \return True if vector empty.
	 */
	bool empty() const
	{
		return m_size == 0;
	}

	/*! Clear vector.
	 *
	 * Clear frees all memory allocated
	 * by the vector.
	 */
	void clear()
	{
		for (size_t i = 0; i < m_size; ++i)
			Constructor::destroy(m_data[i]);

		getAllocator()->free(m_data);

		m_data = nullptr;
		m_size = 0;
		m_capacity = 0;
	}

	/*! Resize vector.
	 *
	 * Allocates more elements if required.
	 * If vector shrink then no reallocation is performed.
	 *
	 * \param size New size of vector.
	 */
	void resize(size_t size)
	{
        ItemType T_ALIGN16 zero = {};
		resize(size, zero);
	}

	/*! Resize vector, pad with given value.
	 *
	 * Allocates more elements if required.
	 * If vector shrink then no reallocation is performed.
	 *
	 * \param size New size of vector.
	 * \param pad Pad value.
	 */
	void resize(size_t size, const ItemType& pad)
	{
		if (size > m_size)
		{
			if (size > m_capacity)
			{
				reserve(size);
				T_ASSERT(size <= m_capacity);
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

	/*! Ensure vector capacity.
	 *
	 * \param capacity Vector capacity.
	 */
	void reserve(size_t capacity)
	{
		if (capacity > m_capacity)
		{
			size_t capacityAlignment = (m_capacity > MinCapacity ? m_capacity : MinCapacity);
			if (capacityAlignment > MaxCapacityAlignment)
				capacityAlignment = MaxCapacityAlignment;

			size_t newCapacity = alignUp(capacity, capacityAlignment);
			ItemType* data = reinterpret_cast< ItemType* >(getAllocator()->alloc(newCapacity * sizeof(ItemType), Alignment, T_FILE_LINE));
			if (m_data)
			{
				for (size_t i = 0; i < m_size; ++i)
					Constructor::move(data[i], m_data[i]);
				getAllocator()->free(m_data);
			}

			m_data = data;
			m_capacity = newCapacity;
		}
	}

	/*! Push default value onto vector.
	 *
	 * \param item Item value.
	 */
	ItemType& push_back()
	{
		grow(1);
		Constructor::construct(m_data[m_size - 1]);
		return m_data[m_size - 1];
	}

	/*! Push value onto vector.
	 *
	 * \param item Item value.
	 */
	void push_back(const ItemType& item)
	{
		grow(1);
		Constructor::construct(m_data[m_size - 1], item);
	}

	/*! Pop value from vector. */
	void pop_back()
	{
		T_ASSERT(m_size > 0);
		Constructor::destroy(m_data[m_size - 1]);
		shrink(1);
	}

	/*! Swap vector content. */
	void swap(AlignedVector< ItemType >& rh)
	{
		{ auto tmp = m_data; m_data = rh.m_data; rh.m_data = tmp; }
		{ auto tmp = m_size; m_size = rh.m_size; rh.m_size = tmp; }
		{ auto tmp = m_capacity; m_capacity = rh.m_capacity; rh.m_capacity = tmp; }
	}

	/*! Get pointer to first element in vector.
	 *
	 * \return Pointer to first element.
	 */
	ItemType* ptr()
	{
		return m_data;
	}

	/*! Get constant pointer to first element in vector.
	 *
	 * \return Pointer to first element.
	 */
	const ItemType* c_ptr() const
	{
		return m_data;
	}

	/*! Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& front()
	{
		T_ASSERT(m_size > 0);
		return *m_data;
	}

	/*! Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& front() const
	{
		T_ASSERT(m_size > 0);
		return *m_data;
	}

	/*! Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& back()
	{
		T_ASSERT(m_size > 0);
		return m_data[m_size - 1];
	}

	/*! Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& back() const
	{
		T_ASSERT(m_size > 0);
		return m_data[m_size - 1];
	}

	/*! Return iterator at first element.
	 *
	 * \return Iterator.
	 */
	iterator begin()
	{
		return iterator(m_data);
	}

	/*! Return reverse iterator.
	 *
	 * \return Iterator.
	 */
	reverse_iterator rbegin()
	{
		return reverse_iterator(&m_data[m_size - 1]);
	}

	/*! Return iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	iterator end()
	{
		return iterator(&m_data[m_size]);
	}

	/*! Return reverse iterator at first element.
	 *
	 * \return Iterator.
	 */
	reverse_iterator rend()
	{
		return reverse_iterator(&m_data[-1]);
	}

	/*! Return constant iterator at first element.
	 *
	 * \return Iterator.
	 */
	const_iterator begin() const
	{
		return const_iterator(m_data);
	}

	/*! Return constant reverse iterator.
	 *
	 * \return Iterator.
	 */
	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(&m_data[m_size - 1]);
	}

	/*! Return constant iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	const_iterator end() const
	{
		return const_iterator(&m_data[m_size]);
	}

	/*! Return constant reverse iterator at first element.
	 *
	 * \return Iterator.
	 */
	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(&m_data[-1]);
	}

	/*! Erase element.
	 *
	 * \param where Iterator at element.
	 * \return New iterator at next element.
	 */
	iterator erase(const iterator& where)
	{
		size_t offset = size_t(where.m_ptr - m_data);

		for (size_t i = offset; i < m_size - 1; ++i)
			move(i, i + 1);

		Constructor::destroy(m_data[m_size - 1]);
		shrink(1);

		return iterator(&m_data[offset]);
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

		size_t offset = size_t(where.m_ptr - m_data);
		size_t count = size_t(last.m_ptr - where.m_ptr);

		if (count > 0)
		{
			for (size_t i = offset; i < m_size - count; ++i)
				move(i, i + count);

			for (size_t i = m_size - count; i < m_size; ++i)
				Constructor::destroy(m_data[i]);

			shrink(count);
		}

		return iterator(&m_data[offset]);
	}

	/*! Insert element into vector.
	 *
	 * \param where Iterator at element.
	 * \param item Item value.
	 * \return Iterator at new element.
	 */
	iterator insert(const iterator& where, const ItemType& item)
	{
		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);

		grow(1);

		// Initialize grown item.
		Constructor::construct(m_data[size]);

		// Move items to make space for new item.
		for (size_t i = size; i > offset; --i)
			move(i, i - 1);

		// Copy insert item into location.
		Constructor::construct(m_data[offset], item);

		return iterator(&m_data[offset]);
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

		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);
		size_t count = size_t(tptr - fptr);

		grow(count);

		// Initialize grown items.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_data[i + size]);

		// Move items to make room for items to be inserted.
		int32_t mv = (int32_t)(size - offset);
		for (int32_t i = mv - 1; i >= 0; --i)
		{
			T_ASSERT(i + offset < size);
			T_ASSERT(i + offset + count < m_size);
			move(i + offset + count, i + offset);
		}

		// Copy insert items into location.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_data[i + offset], fptr[i]);

		return iterator(&m_data[offset]);
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
		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);
		size_t count = size_t(to - from);

		grow(count);

		// Initialize grown items.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_data[i + size]);

		// Move items to make room for items to be inserted.
		int32_t mv = (int32_t)(size - offset);
		for (int32_t i = mv - 1; i >= 0; --i)
		{
			T_ASSERT(i + offset < size);
			T_ASSERT(i + offset + count < m_size);
			move(i + offset + count, i + offset);
		}

		// Copy insert items into location.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_data[i + offset], from[i]);

		return iterator(&m_data[offset]);
	}

	ItemType& operator [] (size_t index)
	{
		T_ASSERT(index < m_size);
		return m_data[index];
	}

	const ItemType& operator [] (size_t index) const
	{
		T_ASSERT(index < m_size);
		return m_data[index];
	}

	AlignedVector< ItemType >& operator = (const AlignedVector< ItemType >& src)
	{
		resize(0);
		insert(begin(), src.begin(), src.end());
		return *this;
	}

	AlignedVector< ItemType >& operator = (AlignedVector< ItemType >&& src) noexcept
	{
		clear();

		m_data = src.m_data;
		m_size = src.m_size;
		m_capacity = src.m_capacity;

		src.m_data = nullptr;
		src.m_size = 0;
		src.m_capacity = 0;

		return *this;
	}

	bool operator == (const AlignedVector< ItemType >& rh) const
	{
		if (m_size != rh.size())
			return false;

		for (size_t i = 0; i < m_size; ++i)
		{
			if ((*this)[i] != rh[i])
				return false;
		}

		return true;
	}

	bool operator != (const AlignedVector< ItemType >& rh) const
	{
		return !(*this == rh);
	}

private:
	ItemType* m_data;
	size_t m_size;
	size_t m_capacity;

	void move(size_t target, size_t source)
	{
		Constructor::move(m_data[target], m_data[source]);
	}

	void grow(size_t count)
	{
		size_t newSize = m_size + count;
		if (newSize > m_capacity)
			reserve(newSize);
		m_size = newSize;
		T_ASSERT(m_size <= m_capacity);
	}

	void shrink(size_t count)
	{
		m_size -= count;
	}
};

}

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
	StaticVector()
	:	m_size(0)
	{
	}

	StaticVector(size_t size)
	:	m_size(size)
	{
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

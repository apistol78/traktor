#ifndef traktor_CircularVector_H
#define traktor_CircularVector_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Circular static container.
 *
 * \note
 * This implementation only permit Capacity-1 elements
 * as one element are unallocated in order to signal full/empty.
 *
 * \ingroup Core
 */
template < typename ItemType, uint32_t Capacity >
class CircularVector
{
public:
	CircularVector()
	:	m_front(0)
	,	m_back(0)
	{
	}

	bool empty() const
	{
		return m_front == m_back;
	}

	bool full() const
	{
		return m_front == ((m_back + 1) % Capacity);
	}

	uint32_t capacity() const
	{
		return Capacity;
	}

	uint32_t size() const
	{
		if (m_front <= m_back)
			return m_back - m_front;
		else
			return (Capacity - m_front) + m_back;
	}

	void clear()
	{
		m_front = 0;
		m_back = 0;
	}

	ItemType& push_back()
	{
		// Get back element.
		ItemType& item = m_items[m_back];
		m_back = (m_back + 1) % Capacity;

		// Discard front element if we've reached front.
		if (m_back == m_front)
			m_front = (m_front + 1) % Capacity;

		return item;
	}

	void push_back(const ItemType& item)
	{
		// Push back element.
		m_items[m_back] = item;
		m_back = (m_back + 1) % Capacity;

		// Discard front element if we've reached front.
		if (m_back == m_front)
			m_front = (m_front + 1) % Capacity;
	}

	void pop_front()
	{
		T_ASSERT (m_front != m_back);
		m_front = (m_front + 1) % Capacity;
	}

	int32_t find(const ItemType& value) const
	{
		for (int32_t i = 0; i < int32_t(size()); ++i)
		{
			if ((*this)[i] == value)
				return i;
		}
		return -1;
	}

	const ItemType& back() const
	{
		return m_items[m_back > 0 ? (m_back - 1) : Capacity - 1];
	}

	const ItemType& front() const
	{
		return m_items[m_front];
	}

	const ItemType& offset(int32_t index) const
	{
		if (index < 0)
			return m_items[(m_back + index) % Capacity];
		else
			return m_items[(m_front + index) % Capacity];
	}

	const ItemType& operator [] (uint32_t index) const
	{
		return m_items[(index + m_front) % Capacity];
	}

	ItemType& operator [] (uint32_t index)
	{
		return m_items[(index + m_front) % Capacity];
	}

private:
	ItemType m_items[Capacity];
	uint32_t m_front;
	uint32_t m_back;
};

}

#endif	// traktor_CircularVector_H

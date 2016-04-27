#ifndef traktor_SmallSet_H
#define traktor_SmallSet_H

#include "Core/Containers/AlignedVector.h"

namespace traktor
{

/*! \brief Small "set" container.
 * \ingroup Core
 *
 * This container is optimized for a set
 * of a few unique items.
 */
template < typename Key >
class SmallSet
{
public:
	typedef typename AlignedVector< Key >::iterator iterator;
	typedef typename AlignedVector< Key >::const_iterator const_iterator;

	iterator begin()
	{
		return m_data.begin();
	}

	const_iterator begin() const
	{
		return m_data.begin();
	}
	
	iterator end()
	{
		return m_data.end();
	}
	
	const_iterator end() const
	{
		return m_data.end();
	}

	void clear()
	{
		m_data.clear();
	}

	bool empty() const
	{
		return m_data.empty();
	}

	iterator find(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return m_data.begin() + i;
		}

		return m_data.end();
	}

	const_iterator find(const Key& value) const
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return m_data.begin() + i;
		}

		return m_data.end();
	}

	bool insert(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
				return false;
		}

		T_ASSERT (is <= m_data.size());
		m_data.insert(m_data.begin() + is, value);
		return true;
	}

	bool erase(const Key& value)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (value < m_data[i])
				ie = i;
			else if (value > m_data[i])
				is = i + 1;
			else if (value == m_data[i])
			{
				m_data.erase(m_data.begin() + i);
				return true;
			}
		}

		return false;
	}

	size_t size() const
	{
		return m_data.size();
	}

	const Key& operator [] (size_t index) const
	{
		return m_data[index];
	}

private:
	AlignedVector< Key > m_data;
};

}

#endif	// traktor_SmallSet_H

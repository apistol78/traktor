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

	bool empty() const
	{
		return m_data.empty();
	}

	iterator find(const Key& value)
	{
		uint32_t is = 0;
		uint32_t ie = uint32_t(m_data.size());

		while (is < ie)
		{
			uint32_t i = (is + ie) >> 1;
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
		uint32_t is = 0;
		uint32_t ie = uint32_t(m_data.size());

		while (is < ie)
		{
			uint32_t i = (is + ie) >> 1;
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
		uint32_t is = 0;
		uint32_t ie = uint32_t(m_data.size());

		while (is < ie)
		{
			uint32_t i = (is + ie) >> 1;
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
		uint32_t is = 0;
		uint32_t ie = uint32_t(m_data.size());

		while (is < ie)
		{
			uint32_t i = (is + ie) >> 1;
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

private:
	AlignedVector< Key > m_data;
};

}

#endif	// traktor_SmallSet_H

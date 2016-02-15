#ifndef traktor_SmallMap_H
#define traktor_SmallMap_H

#include "Core/Containers/AlignedVector.h"

namespace traktor
{

/*! \brief Small directional map
 * \ingroup Core
 *
 * This container is optimized for fast lookup
 * of a small, ~10-20, set of items.
 * Lookup is performed in a sorted, linear
 * array of pairs using simple binary search.
 */
template < typename Key, typename Item >
class SmallMap
{
public:
	typedef std::pair< Key, Item > pair_t;
	typedef typename AlignedVector< pair_t >::iterator iterator;
	typedef typename AlignedVector< pair_t >::const_iterator const_iterator;

	void reserve(size_t capacity)
	{
		m_data.reserve(capacity);
	}

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
	
	iterator find(const Key& key)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data.begin() + i;
		}

		return end();
	}
	
	const_iterator find(const Key& key) const
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data.begin() + i;
		}

		return end();
	}
	
	void insert(const pair_t& pair)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (pair.first < m_data[i].first)
				ie = i;
			else if (pair.first > m_data[i].first)
				is = i + 1;
			else if (pair.first == m_data[i].first)
				return;
		}

		T_ASSERT (is <= m_data.size());
		m_data.insert(m_data.begin() + is, pair);
	}

	void insert(const Key& key, const Item& item)
	{
		insert(std::make_pair(key, item));
	}

	void insert(const const_iterator& first, const const_iterator& last)
	{
		for (const_iterator i = first; i != last; ++i)
			insert(*i);
	}
	
	iterator erase(const iterator& it)
	{
		return m_data.erase(it);
	}

	iterator erase(const iterator& first, const iterator& last)
	{
		return m_data.erase(first, last);
	}

	void remove(const Key& key)
	{
		iterator it = find(key);
		if (it != end())
			erase(it);
	}

	/*! \brief Reset content but keep memory allocated.
	 * This method reset the map's content but keeps memory
	 * allocated for it's internal vector to quickly be
	 * able to rebuild map without reallocating memory.
	 */
	void reset()
	{
		m_data.resize(0);
	}

	/*! \brief Clear content and release allocated memory. */
	void clear()
	{
		m_data.clear();
	}

	bool empty() const
	{
		return m_data.empty();
	}

	size_t size() const
	{
		return m_data.size();
	}

	Item& operator [] (const Key& key)
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data[i].second;
		}

		m_data.insert(m_data.begin() + is, std::make_pair(key, Item()));
		return m_data[is].second;
	}

	Item operator [] (const Key& key) const
	{
		size_t is = 0;
		size_t ie = m_data.size();

		while (is < ie)
		{
			size_t i = (is + ie) >> 1;
			if (key < m_data[i].first)
				ie = i;
			else if (key > m_data[i].first)
				is = i + 1;
			else if (key == m_data[i].first)
				return m_data[i].second;
		}

		return Item();
	}
	
private:
	AlignedVector< pair_t > m_data;
};

}

#endif	// traktor_SmallMap_H

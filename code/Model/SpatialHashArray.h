#ifndef traktor_model_SpatialHashArray_H
#define traktor_model_SpatialHashArray_H

#include <map>
#include <set>
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/Adler32.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace model
	{

//@{
// \ingroup Model

template < typename ItemType >
struct SpatialHashFunction
{
	static uint32_t calculateHash(const ItemType& v)
	{
		Adler32 adler;
		adler.begin();
		adler.feed(&v, sizeof(v));
		adler.end();
		return adler.get();
	}
};

/*! \brief Array of vectors, using a spatial hash to optimize index lookups.
 * \ingroup Model
 */
template < typename ItemType, typename ItemHash = SpatialHashFunction< ItemType > >
class SpatialHashArray
{
public:
	static const uint32_t InvalidIndex = ~0U;

	template < typename ItemPredicate >
	uint32_t get(const ItemType& v, const ItemPredicate& predicate = ItemPredicate()) const
	{
		uint32_t hash = ItemHash::calculateHash(v);

		std::map< uint32_t, std::set< uint32_t > >::const_iterator it = m_indices.find(hash);
		if (it != m_indices.end())
		{
			for (std::set< uint32_t >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
			{
				if ((predicate)(m_items[*i], v))
					return *i;
			}
		}

		return InvalidIndex;
	}

	void reserve(size_t capacity)
	{
		m_items.reserve(capacity);
	}

	uint32_t add(const ItemType& v)
	{
		uint32_t id = uint32_t(m_items.size());
		m_items.push_back(v);

		uint32_t hash = ItemHash::calculateHash(v);
		m_indices[hash].insert(id);

		return id;
	}

	void clear()
	{
		m_indices.clear();
		m_items.resize(0);
	}

	void replace(const AlignedVector< ItemType >& items)
	{
		m_items = items;
		m_indices.clear();
		for (uint32_t i = 0; i < uint32_t(items.size()); ++i)
		{
			uint32_t hash = ItemHash::calculateHash(items[i]);
			m_indices[hash].insert(i);
		}
	}

	void set(uint32_t index, const ItemType& item)
	{
		uint32_t hash1 = ItemHash::calculateHash(m_items[index]);
		m_indices[hash1].erase(index);

		uint32_t hash2 = ItemHash::calculateHash(item);
		m_items[index] = item;
		m_indices[hash2].insert(index);
	}

	const ItemType& get(uint32_t index) const
	{
		return m_items[index];
	}

	const AlignedVector< ItemType >& items() const
	{
		return m_items;
	}

private:
	std::map< uint32_t, std::set< uint32_t > > m_indices;
	AlignedVector< ItemType > m_items;
};

//@}

	}
}

#endif	// traktor_model_SpatialHashArray_H

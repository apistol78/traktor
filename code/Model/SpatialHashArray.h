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

/*! \brief
 * \ingroup Model
 */
template < typename ItemType >
class SpatialHashArray
{
public:
	static const uint32_t InvalidIndex = ~0U;

	template < typename ItemPredicate >
	uint32_t get(const ItemType& v, const ItemPredicate& predicate = ItemPredicate()) const
	{
		for (uint32_t i = 0; i < m_items.size(); ++i)
		{
			if (predicate(m_items[i], v))
				return i;
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
		return id;
	}

	void clear()
	{
		m_items.resize(0);
	}

	void replace(const AlignedVector< ItemType >& items)
	{
		m_items = items;
	}

	void set(uint32_t index, const ItemType& item)
	{
		m_items[index] = item;
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
	AlignedVector< ItemType > m_items;
};

//@}

	}
}

#endif	// traktor_model_SpatialHashArray_H

#ifndef traktor_model_Grid2_H
#define traktor_model_Grid2_H

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace model
	{

/*! \brief Default accessor of position from value.
 * \ingroup Model
 */
template < typename ValueType >
struct DefaultPositionAccessor2
{
	static Vector2 get(const ValueType& v) { return v; }
};

/*! \brief Default hash function.
 * \ingroup Model
 */
struct DefaultHashFunction2
{
	enum
	{
		Prim1 = 73856093UL,
		Prim2 = 19349663UL
	};

	static uint32_t get(int32_t x, int32_t y)
	{
		return uint32_t((x * Prim1) ^ (y * Prim2));
	}
};

/*! \brief 2-dimensional grid container.
 * \ingroup Model
 */
template
<
	typename ValueType,
	typename PositionAccessor = DefaultPositionAccessor2< ValueType >,
	typename HashFunction = DefaultHashFunction2
>
class Grid2
{
public:
	static const uint32_t InvalidIndex = ~0U;

	Grid2(float cellSize)
	:	m_cellSize(cellSize)
	{
	}

	const ValueType& get(uint32_t index) const
	{
		return m_values[index];
	}

	uint32_t get(const ValueType& v, float distance) const
	{
		T_ASSERT (distance <= m_cellSize / 2.0f);

		Vector2 p = PositionAccessor::get(v);
		Vector2 pq = p / m_cellSize;

		int32_t x = int32_t(pq.x);
		int32_t y = int32_t(pq.y);

		for (int32_t iy = -1; iy <= 1; ++iy)
		{
			for (int32_t ix = -1; ix <= 1; ++ix)
			{
				uint32_t hash = HashFunction::get(x + ix, y + iy);

				SmallMap< uint32_t, AlignedVector< uint32_t > >::const_iterator i = m_indices.find(hash);
				if (i == m_indices.end())
					continue;

				const AlignedVector< uint32_t >& indices = i->second;
				for (AlignedVector< uint32_t >::const_iterator j = indices.begin(); j != indices.end(); ++j)
				{
					Vector2 pv = PositionAccessor::get(m_values[*j]);
					if ((pv - p).length2() <= distance * distance)
						return *j;
				}
			}
		}

		return InvalidIndex;
	}

	uint32_t add(const ValueType& v)
	{
		Vector2 p = PositionAccessor::get(v) / m_cellSize;

		int32_t x = int32_t(p.x);
		int32_t y = int32_t(p.y);

		uint32_t hash = HashFunction::get(x, y);
		AlignedVector< uint32_t >& indices = m_indices[hash];

		uint32_t id = uint32_t(m_values.size());
		m_values.push_back(v);

		indices.push_back(id);

		return id;
	}

	void replace(const AlignedVector< ValueType >& values)
	{
		m_values = values;
		m_indices.clear();

		for (uint32_t i = 0; i < uint32_t(m_values.size()); ++i)
		{
			Vector2 p = PositionAccessor::get(m_values[i]) / m_cellSize;

			int32_t x = int32_t(p.x);
			int32_t y = int32_t(p.y);

			uint32_t hash = HashFunction::get(x, y);
			m_indices[hash].push_back(i);
		}
	}

	void clear()
	{
		m_indices.clear();
		m_values.clear();
	}

	void reserve(size_t capacity)
	{
		m_values.reserve(capacity);
	}

	const AlignedVector< ValueType >& values() const
	{
		return m_values;
	}

private:
	float m_cellSize;
	SmallMap< uint32_t, AlignedVector< uint32_t > > m_indices;
	AlignedVector< ValueType > m_values;
};

	}
}

#endif	// traktor_model_Grid2_H

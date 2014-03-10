#ifndef traktor_model_Grid3_H
#define traktor_model_Grid3_H

#include <map>
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace model
	{

/*! \brief Default accessor of position from value.
 * \ingroup Model
 */
template < typename ValueType >
struct DefaultPositionAccessor3
{
	static Vector4 get(const ValueType& v) { return v; }
};

/*! \brief Default hash function.
 * \ingroup Model
 */
struct DefaultHashFunction3
{
	enum
	{
		Prim1 = 73856093UL,
		Prim2 = 19349663UL,
		Prim3 = 83492791UL
	};

	static uint32_t get(int32_t x, int32_t y, int32_t z)
	{
		return uint32_t((x * Prim1) ^ (y * Prim2) ^ (z * Prim3));
	}
};

/*! \brief 3-dimensional grid container.
 * \ingroup Model
 */
template
<
	typename ValueType,
	typename PositionAccessor = DefaultPositionAccessor3< ValueType >,
	typename HashFunction = DefaultHashFunction3
>
class Grid3
{
public:
	static const uint32_t InvalidIndex = ~0U;

	Grid3(float cellSize)
	:	m_cellSize(cellSize)
	{
	}

	const ValueType& get(uint32_t index) const
	{
		return m_values[index];
	}

	void set(uint32_t index, const ValueType& v)
	{
		// Remove index from cell.
		{
			Vector4 p = PositionAccessor::get(m_values[index]) / m_cellSize;

			int32_t x = int32_t(std::floor(p.x()));
			int32_t y = int32_t(std::floor(p.y()));
			int32_t z = int32_t(std::floor(p.z()));

			uint32_t hash = HashFunction::get(x, y, z);
			AlignedVector< uint32_t >& indices = m_indices[hash];
			AlignedVector< uint32_t >::iterator it = std::find(indices.begin(), indices.end(), index);
			if (it != indices.end())
				indices.erase(it);
		}

		// Add index to new cell.
		{
			Vector4 p = PositionAccessor::get(m_values[index]) / m_cellSize;

			int32_t x = int32_t(std::floor(p.x()));
			int32_t y = int32_t(std::floor(p.y()));
			int32_t z = int32_t(std::floor(p.z()));

			uint32_t hash = HashFunction::get(x, y, z);
			m_indices[hash].push_back(index);
		}

		// Modify value.
		m_values[index] = v;
	}

	uint32_t get(const ValueType& v, float distance) const
	{
		T_ASSERT (distance <= m_cellSize / 2.0f);

		Vector4 p = PositionAccessor::get(v);
		Vector4 pq = p / m_cellSize;

		int32_t mnx = int32_t(std::floor(pq.x() - distance - 0.5f));
		int32_t mny = int32_t(std::floor(pq.y() - distance - 0.5f));
		int32_t mnz = int32_t(std::floor(pq.z() - distance - 0.5f));

		int32_t mxx = int32_t(std::floor(pq.x() + distance + 0.5f));
		int32_t mxy = int32_t(std::floor(pq.y() + distance + 0.5f));
		int32_t mxz = int32_t(std::floor(pq.z() + distance + 0.5f));

		for (int32_t iz = mnz; iz <= mxz; ++iz)
		{
			for (int32_t iy = mny; iy <= mxy; ++iy)
			{
				for (int32_t ix = mnx; ix <= mxx; ++ix)
				{
					uint32_t hash = HashFunction::get(ix, iy, iz);

					std::map< uint32_t, AlignedVector< uint32_t > >::const_iterator i = m_indices.find(hash);
					if (i == m_indices.end())
						continue;

					const AlignedVector< uint32_t >& indices = i->second;
					for (AlignedVector< uint32_t >::const_iterator j = indices.begin(); j != indices.end(); ++j)
					{
						Vector4 pv = PositionAccessor::get(m_values[*j]);
						if ((pv - p).length2() <= distance * distance)
							return *j;
					}
				}
			}
		}

		return InvalidIndex;
	}

	uint32_t add(const ValueType& v)
	{
		Vector4 p = PositionAccessor::get(v) / m_cellSize;

		int32_t x = int32_t(std::floor(p.x()));
		int32_t y = int32_t(std::floor(p.y()));
		int32_t z = int32_t(std::floor(p.z()));

		uint32_t hash = HashFunction::get(x, y, z);
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

		for (size_t i = 0; i < m_values.size(); ++i)
		{
			Vector4 p = PositionAccessor::get(m_values[i]) / m_cellSize;

			int32_t x = int32_t(std::floor(p.x()));
			int32_t y = int32_t(std::floor(p.y()));
			int32_t z = int32_t(std::floor(p.z()));

			uint32_t hash = HashFunction::get(x, y, z);
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

	uint32_t size() const
	{
		return m_values.size();
	}

	const AlignedVector< ValueType >& values() const
	{
		return m_values;
	}

private:
	Scalar m_cellSize;
	std::map< uint32_t, AlignedVector< uint32_t > > m_indices;
	AlignedVector< ValueType > m_values;
};

	}
}

#endif	// traktor_model_Grid3_H

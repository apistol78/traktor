#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace model
	{

/*! Default accessor of position from value.
 * \ingroup Model
 */
template < typename ValueType >
struct DefaultPositionAccessor3
{
	static const Vector4& get(const ValueType& v) { return v; }
};

/*! Default hash function.
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

/*! 3-dimensional grid container.
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

	explicit Grid3(float cellSize)
	:	m_cellSize(cellSize)
	{
	}

	const ValueType& get(uint32_t index, const ValueType& defaultValue = ValueType()) const
	{
		return index < m_values.size() ? m_values[index] : defaultValue;
	}

	void set(uint32_t index, const ValueType& v)
	{
		// Remove index from cell.
		{
			const Vector4 p = PositionAccessor::get(m_values[index]) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);

			auto& indices = m_indices[hash];
			auto it = std::remove(indices.begin(), indices.end(), index);
			indices.erase(it, indices.end());
		}

		// Add index to new cell.
		{
			const Vector4 p = PositionAccessor::get(v) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);
			m_indices[hash].push_back(index);
		}

		// Modify value.
		m_values[index] = v;
	}

	uint32_t get(const ValueType& v, float distance) const
	{
		T_ASSERT(distance <= m_cellSize / 2.0f);

		const Vector4 p = PositionAccessor::get(v);
		const Vector4 pq = p / m_cellSize;

		const Scalar sd(distance);
		const Vector4 mnpq = pq - sd - 0.5_simd;
		const Vector4 mxpq = pq + sd + 0.5_simd;

		T_MATH_ALIGN16 int32_t mne[4];
		mnpq.storeIntegersAligned(mne);

		T_MATH_ALIGN16 int32_t mxe[4];
		mxpq.storeIntegersAligned(mxe);

		const int32_t mnx = mne[0];
		const int32_t mny = mne[1];
		const int32_t mnz = mne[2];
		const int32_t mxx = mxe[0];
		const int32_t mxy = mxe[1];
		const int32_t mxz = mxe[2];

		for (int32_t iz = mnz; iz <= mxz; ++iz)
		{
			for (int32_t iy = mny; iy <= mxy; ++iy)
			{
				for (int32_t ix = mnx; ix <= mxx; ++ix)
				{
					const uint32_t hash = HashFunction::get(ix, iy, iz);

					auto it = m_indices.find(hash);
					if (it == m_indices.end())
						continue;

					for (auto index : it->second)
					{
						Vector4 pv = PositionAccessor::get(m_values[index]);
						if ((pv - p).length2() <= distance * distance)
							return index;
					}
				}
			}
		}

		return InvalidIndex;
	}

	uint32_t add(const ValueType& v)
	{
		Vector4 p = PositionAccessor::get(v) / m_cellSize;

		T_MATH_ALIGN16 int32_t pe[4];
		p.storeIntegersAligned(pe);

		const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);
		const uint32_t id = (uint32_t)m_values.size();

		m_values.push_back(v);
		m_indices[hash].push_back(id);
		return id;
	}

	void swap(AlignedVector< ValueType >& values)
	{
		m_values.swap(values);
		m_indices.reset();
		m_indices.reserve(m_values.size());
		for (uint32_t i = 0; i < (uint32_t)m_values.size(); ++i)
		{
			const Vector4 p = PositionAccessor::get(m_values[i]) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);
			m_indices[hash].push_back(i);
		}
	}

	void replace(const AlignedVector< ValueType >& values)
	{
		m_values = values;
		m_indices.reset();
		m_indices.reserve(m_values.size());
		for (uint32_t i = 0; i < (uint32_t)m_values.size(); ++i)
		{
			const Vector4 p = PositionAccessor::get(m_values[i]) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);
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
		return uint32_t(m_values.size());
	}

	const AlignedVector< ValueType >& values() const
	{
		return m_values;
	}

private:
	Scalar m_cellSize;
	SmallMap< uint32_t, AlignedVector< uint32_t > > m_indices;
	AlignedVector< ValueType > m_values;
};

	}
}


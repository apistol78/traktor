/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector4.h"

namespace traktor::model
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
	static constexpr uint32_t InvalidIndex = ~0U;
	static constexpr uint32_t HashBuckets = 256;

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
		uint32_t fromHash, toHash;

		// Calculate cell hashes.
		{
			const Vector4 p = PositionAccessor::get(m_values[index]) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			fromHash = HashFunction::get(pe[0], pe[1], pe[2]);
		}
		{
			const Vector4 p = PositionAccessor::get(v) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			toHash = HashFunction::get(pe[0], pe[1], pe[2]);
		}

		// Remove index from cell and add to new cell.
		if (fromHash != toHash)
		{
			auto& indices = m_indices[fromHash & (HashBuckets - 1)];
			if (!indices.empty())
			{
				auto it = std::remove(indices.begin(), indices.end(), index);
				indices.erase(it, indices.end());
			}

			m_indices[toHash & (HashBuckets - 1)].push_back(index);
		}

		// Modify value.
		m_values[index] = v;
	}

	uint32_t get(const ValueType& v) const
	{
		const Vector4 p = PositionAccessor::get(v);
		const Vector4 pq = p / m_cellSize;

		T_MATH_ALIGN16 int32_t ipq[4];
		pq.storeIntegersAligned(ipq);

		const uint32_t hash = HashFunction::get(ipq[0], ipq[1], ipq[2]);
		for (auto index : m_indices[hash & (HashBuckets - 1)])
		{
			const Vector4 pv = PositionAccessor::get(m_values[index]);
			if ((pv - p).length2() <= Scalar(FUZZY_EPSILON))
				return index;
		}

		return InvalidIndex;
	}

	uint32_t get(const ValueType& v, const Scalar& distance) const
	{
		T_ASSERT(distance <= m_cellSize / 2.0_simd);

		const Scalar distance2 = distance * distance;

		const Vector4 p = PositionAccessor::get(v);
		const Vector4 pq = p / m_cellSize;

		const Vector4 mnpq = pq - distance - 0.5_simd;
		const Vector4 mxpq = pq + distance + 0.5_simd;

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
					for (auto index : m_indices[hash & (HashBuckets - 1)])
					{
						const Vector4 pv = PositionAccessor::get(m_values[index]);
						if ((pv - p).length2() <= distance2)
							return index;
					}
				}
			}
		}

		return InvalidIndex;
	}

	uint32_t add(const ValueType& v)
	{
		const Vector4 p = PositionAccessor::get(v) / m_cellSize;

		T_MATH_ALIGN16 int32_t pe[4];
		p.storeIntegersAligned(pe);

		const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);
		const uint32_t id = (uint32_t)m_values.size();

		m_values.push_back(v);
		m_indices[hash & (HashBuckets - 1)].push_back(id);
		return id;
	}

	void swap(AlignedVector< ValueType >& values)
	{
		m_values.swap(values);
		rehash();
	}

	void replace(const AlignedVector< ValueType >& values)
	{
		m_values = values;
		rehash();
	}

	void clear()
	{
		m_values.clear();
		for (uint32_t i = 0; i < HashBuckets; ++i)
			m_indices[i].clear();
	}

	void reserve(size_t capacity)
	{
		m_values.reserve(capacity);
	}

	uint32_t size() const
	{
		return (uint32_t)m_values.size();
	}

	const AlignedVector< ValueType >& values() const
	{
		return m_values;
	}

private:
	AlignedVector< uint32_t > m_indices[HashBuckets];
	AlignedVector< ValueType > m_values;
	Scalar m_cellSize;

	void rehash()
	{
		for (uint32_t i = 0; i < HashBuckets; ++i)
			m_indices[i].resize(0);

		for (uint32_t i = 0; i < (uint32_t)m_values.size(); ++i)
		{
			const Vector4 p = PositionAccessor::get(m_values[i]) / m_cellSize;

			T_MATH_ALIGN16 int32_t pe[4];
			p.storeIntegersAligned(pe);

			const uint32_t hash = HashFunction::get(pe[0], pe[1], pe[2]);
			m_indices[hash & (HashBuckets - 1)].push_back(i);
		}
	}
};

}

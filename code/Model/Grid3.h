/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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
 *
 * Spatial hash for fast proximity queries over 3D values. Cells are
 * floor-quantised and indexed into a power-of-two bucket array.
 * \p HashBuckets is the bucket count and trades memory for fewer hash
 * collisions; bump it for large grids.
 */
template
<
	typename ValueType,
	typename PositionAccessor = DefaultPositionAccessor3< ValueType >,
	typename HashFunction = DefaultHashFunction3,
	uint32_t HashBuckets = 4096
>
class Grid3
{
public:
	static constexpr uint32_t InvalidIndex = ~0U;
	static_assert((HashBuckets & (HashBuckets - 1)) == 0, "HashBuckets must be a power of two.");

	explicit Grid3(float cellSize)
	:	m_cellSize(cellSize)
	,	m_invCellSize(1.0f / cellSize)
	{
		// Bucket array lives on the heap — inline storage would balloon the
		// containing object (Model embeds several grids) and overflow the stack
		// of any function that holds a Grid3 by value.
		m_indices.resize(HashBuckets);
	}

	const ValueType& get(uint32_t index, const ValueType& defaultValue = ValueType()) const
	{
		return index < m_values.size() ? m_values[index] : defaultValue;
	}

	void set(uint32_t index, const ValueType& v)
	{
		T_MATH_ALIGN16 int32_t fc[4];
		cellOf(PositionAccessor::get(m_values[index]), fc);
		const uint32_t fromBucket = HashFunction::get(fc[0], fc[1], fc[2]) & (HashBuckets - 1);

		const Vector4 newPos = PositionAccessor::get(v);
		T_MATH_ALIGN16 int32_t tc[4];
		cellOf(newPos, tc);
		const uint32_t toBucket = HashFunction::get(tc[0], tc[1], tc[2]) & (HashBuckets - 1);

		if (fromBucket != toBucket)
		{
			auto& indices = m_indices[fromBucket];
			for (size_t k = 0; k < indices.size(); ++k)
			{
				if (indices[k] == index)
				{
					indices[k] = indices.back();
					indices.pop_back();
					break;
				}
			}
			m_indices[toBucket].push_back(index);
		}

		m_values[index] = v;
	}

	uint32_t get(const ValueType& v) const
	{
		const Vector4 p = PositionAccessor::get(v);
		T_MATH_ALIGN16 int32_t c[4];
		cellOf(p, c);

		const uint32_t hash = HashFunction::get(c[0], c[1], c[2]);
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
		T_MATH_ALIGN16 int32_t c[4];
		cellOf(p, c);

		// distance <= cellSize/2 guarantees any candidate is within the
		// 3x3x3 neighbourhood — no need to compute a wider span.
		for (int32_t iz = -1; iz <= 1; ++iz)
		{
			for (int32_t iy = -1; iy <= 1; ++iy)
			{
				for (int32_t ix = -1; ix <= 1; ++ix)
				{
					const uint32_t hash = HashFunction::get(c[0] + ix, c[1] + iy, c[2] + iz);
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
		const Vector4 p = PositionAccessor::get(v);
		T_MATH_ALIGN16 int32_t c[4];
		cellOf(p, c);

		const uint32_t hash = HashFunction::get(c[0], c[1], c[2]);
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
	AlignedVector< AlignedVector< uint32_t > > m_indices;
	AlignedVector< ValueType > m_values;
	Scalar m_cellSize;
	Scalar m_invCellSize;

	// Compute integer cell coordinates for a world position, using floor()
	// so cells are symmetric around the origin (no fat cell at zero).
	void cellOf(const Vector4& p, int32_t out[4]) const
	{
		(p * m_invCellSize).floor().storeIntegersAligned(out);
	}

	void rehash()
	{
		for (uint32_t i = 0; i < HashBuckets; ++i)
			m_indices[i].resize(0);

		for (uint32_t i = 0; i < (uint32_t)m_values.size(); ++i)
		{
			const Vector4 p = PositionAccessor::get(m_values[i]);
			T_MATH_ALIGN16 int32_t c[4];
			cellOf(p, c);

			const uint32_t hash = HashFunction::get(c[0], c[1], c[2]);
			m_indices[hash & (HashBuckets - 1)].push_back(i);
		}
	}
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cmath>
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"

namespace traktor::model
{

/*! Default accessor of position from value.
 * \ingroup Model
 */
template < typename ValueType >
struct DefaultPositionAccessor2
{
	static Vector2 get(const ValueType& v) { return v; }
};

/*! Default hash function.
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

/*! 2-dimensional grid container.
 * \ingroup Model
 *
 * Spatial hash for fast proximity queries over 2D values. Cells are
 * floor-quantised (symmetric around zero — no fat cell at the origin) and
 * indexed into a power-of-two bucket array. \p HashBuckets is the bucket
 * count and trades memory for fewer hash collisions; bump it for large grids.
 */
template
<
	typename ValueType,
	typename PositionAccessor = DefaultPositionAccessor2< ValueType >,
	typename HashFunction = DefaultHashFunction2,
	uint32_t HashBuckets = 4096
>
class Grid2
{
public:
	static constexpr uint32_t InvalidIndex = ~0U;
	static_assert((HashBuckets & (HashBuckets - 1)) == 0, "HashBuckets must be a power of two.");

	explicit Grid2(float cellSize)
	:	m_cellSize(cellSize)
	,	m_invCellSize(1.0f / cellSize)
	{
		// Bucket array lives on the heap — inline storage would balloon the
		// containing object (Model embeds several grids) and overflow the stack
		// of any function that holds a Grid2 by value.
		m_indices.resize(HashBuckets);
	}

	const ValueType& get(uint32_t index, const ValueType& defaultValue = ValueType()) const
	{
		return index < m_values.size() ? m_values[index] : defaultValue;
	}

	uint32_t get(const ValueType& v) const
	{
		const Vector2 p = PositionAccessor::get(v);
		const int32_t x = floorToInt(p.x * m_invCellSize);
		const int32_t y = floorToInt(p.y * m_invCellSize);

		const uint32_t hash = HashFunction::get(x, y);
		for (auto index : m_indices[hash & (HashBuckets - 1)])
		{
			const Vector2 pv = PositionAccessor::get(m_values[index]);
			if ((pv - p).length2() <= 1e-8f)
				return index;
		}

		return InvalidIndex;
	}

	uint32_t get(const ValueType& v, float distance) const
	{
		T_ASSERT(distance <= m_cellSize / 2.0f);
		const float distance2 = distance * distance;

		const Vector2 p = PositionAccessor::get(v);
		const int32_t cx = floorToInt(p.x * m_invCellSize);
		const int32_t cy = floorToInt(p.y * m_invCellSize);

		for (int32_t iy = -1; iy <= 1; ++iy)
		{
			for (int32_t ix = -1; ix <= 1; ++ix)
			{
				const uint32_t hash = HashFunction::get(cx + ix, cy + iy);
				for (auto index : m_indices[hash & (HashBuckets - 1)])
				{
					const Vector2 pv = PositionAccessor::get(m_values[index]);
					if ((pv - p).length2() <= distance2)
						return index;
				}
			}
		}

		return InvalidIndex;
	}

	uint32_t add(const ValueType& v)
	{
		const Vector2 p = PositionAccessor::get(v);
		const int32_t x = floorToInt(p.x * m_invCellSize);
		const int32_t y = floorToInt(p.y * m_invCellSize);
		const uint32_t hash = HashFunction::get(x, y);
		const uint32_t id = (uint32_t)m_values.size();
		m_values.push_back(v);
		m_indices[hash & (HashBuckets - 1)].push_back(id);
		return id;
	}

	void set(uint32_t index, const ValueType& v)
	{
		const Vector2 oldPos = PositionAccessor::get(m_values[index]);
		const int32_t fromX = floorToInt(oldPos.x * m_invCellSize);
		const int32_t fromY = floorToInt(oldPos.y * m_invCellSize);
		const uint32_t fromBucket = HashFunction::get(fromX, fromY) & (HashBuckets - 1);

		const Vector2 newPos = PositionAccessor::get(v);
		const int32_t toX = floorToInt(newPos.x * m_invCellSize);
		const int32_t toY = floorToInt(newPos.y * m_invCellSize);
		const uint32_t toBucket = HashFunction::get(toX, toY) & (HashBuckets - 1);

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
	float m_cellSize;
	float m_invCellSize;

	static int32_t floorToInt(float v)
	{
		return (int32_t)std::floor(v);
	}

	void rehash()
	{
		for (uint32_t i = 0; i < HashBuckets; ++i)
			m_indices[i].resize(0);

		for (uint32_t i = 0; i < (uint32_t)m_values.size(); ++i)
		{
			const Vector2 p = PositionAccessor::get(m_values[i]);
			const int32_t x = floorToInt(p.x * m_invCellSize);
			const int32_t y = floorToInt(p.y * m_invCellSize);
			const uint32_t hash = HashFunction::get(x, y);
			m_indices[hash & (HashBuckets - 1)].push_back(i);
		}
	}
};

}

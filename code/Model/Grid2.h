/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
	static constexpr uint32_t InvalidIndex = ~0U;
	static constexpr uint32_t HashBuckets = 256;

	explicit Grid2(float cellSize)
	:	m_cellSize(cellSize)
	{
	}

	const ValueType& get(uint32_t index, const ValueType& defaultValue = ValueType()) const
	{
		return index < m_values.size() ? m_values[index] : defaultValue;
	}

	uint32_t get(const ValueType& v) const
	{
		const Vector2 p = PositionAccessor::get(v);
		const Vector2 pq = p / m_cellSize;

		const int32_t x = (int32_t)pq.x;
		const int32_t y = (int32_t)pq.y;

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

		const Vector2 p = PositionAccessor::get(v);
		const Vector2 pq = p / m_cellSize;

		const int32_t x = (int32_t)pq.x;
		const int32_t y = (int32_t)pq.y;

		for (int32_t iy = -1; iy <= 1; ++iy)
		{
			for (int32_t ix = -1; ix <= 1; ++ix)
			{
				const uint32_t hash = HashFunction::get(x + ix, y + iy);
				for (auto index : m_indices[hash & (HashBuckets - 1)])
				{
					const Vector2 pv = PositionAccessor::get(m_values[index]);
					if ((pv - p).length2() <= distance * distance)
						return index;
				}
			}
		}

		return InvalidIndex;
	}

	uint32_t add(const ValueType& v)
	{
		const Vector2 p = PositionAccessor::get(v) / m_cellSize;
		const int32_t x = (int32_t)p.x;
		const int32_t y = (int32_t)p.y;
		const uint32_t hash = HashFunction::get(x, y);
		AlignedVector< uint32_t >& indices = m_indices[hash & (HashBuckets - 1)];
		const uint32_t id = (uint32_t)m_values.size();
		m_values.push_back(v);
		indices.push_back(id);
		return id;
	}

	void swap(AlignedVector< ValueType >& values)
	{
		m_values.swap(values);

		for (uint32_t i = 0; i < HashBuckets; ++i)
			m_indices[i].resize(0);

		for (uint32_t i = 0; i < uint32_t(m_values.size()); ++i)
		{
			const Vector2 p = PositionAccessor::get(m_values[i]) / m_cellSize;
			const int32_t x = (int32_t)p.x;
			const int32_t y = (int32_t)p.y;
			const uint32_t hash = HashFunction::get(x, y);
			m_indices[hash & (HashBuckets - 1)].push_back(i);
		}
	}

	void replace(const AlignedVector< ValueType >& values)
	{
		m_values = values;
		
		for (uint32_t i = 0; i < HashBuckets; ++i)
			m_indices[i].resize(0);

		for (uint32_t i = 0; i < uint32_t(m_values.size()); ++i)
		{
			const Vector2 p = PositionAccessor::get(m_values[i]) / m_cellSize;
			const int32_t x = (int32_t)p.x;
			const int32_t y = (int32_t)p.y;
			const uint32_t hash = HashFunction::get(x, y);
			m_indices[hash & (HashBuckets - 1)].push_back(i);
		}
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

	const AlignedVector< ValueType >& values() const
	{
		return m_values;
	}

private:
	AlignedVector< uint32_t > m_indices[HashBuckets];
	AlignedVector< ValueType > m_values;
	float m_cellSize;
};

}

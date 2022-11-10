/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include <functional>
#include "Core/Math/Aabb3.h"

namespace traktor
{
	namespace shape
	{

template < typename ItemType >
struct OctTreePositionAccessor
{
	static const Vector4& get(const ItemType& item) { return item; }
};

template < typename ItemType, typename PositionAccessor = OctTreePositionAccessor< ItemType > >
class Grid3
{
public:
	static constexpr int32_t MaxItemsPerNode = 4;

	struct Grid
	{
		int32_t x;
		int32_t y;
		int32_t z;
	};

	struct Node
	{
		ItemType items[MaxItemsPerNode];
		std::atomic< int32_t > size;

		Node()
		:	size(0)
		{
		}
	};

	explicit Grid3(const Aabb3& bounds, float searchRadius)
	:	m_bounds(bounds)
	,	m_searchRadius(searchRadius)
	{
		Grid gmn = calculateGrid(bounds.mn);
		Grid gmx = calculateGrid(bounds.mx);
		m_gridDim = { gmx.x - gmn.x + 1, gmx.y - gmn.y + 1, gmx.z - gmn.z + 1 };
		uint32_t nodeCount = m_gridDim.x * m_gridDim.y * m_gridDim.z;
		m_nodes = new Node [nodeCount];
	}

	virtual ~Grid3()
	{
		delete m_nodes;
	}

	void insert(const ItemType& item)
	{
		Vector4 pt = PositionAccessor::get(item);
		T_ASSERT(m_bounds.inside(pt));

		Grid gpt = calculateGrid(pt);
		uint32_t index = gridNodeIndex(gpt);

		auto& node = m_nodes[index];
		if (node.size >= MaxItemsPerNode)
			return;

		int32_t i = node.size;
		node.items[i] = item;
		node.size = i + 1;
	}

	void get(const Vector4& pt, const std::function< void(const ItemType*, int32_t) >& fn) const
	{
		Grid gpt = calculateGrid(pt);
		Grid gmn = { std::max(gpt.x - 1, 0), std::max(gpt.y - 1, 0), std::max(gpt.z - 1, 0) };
		Grid gmx = { std::min(gpt.x + 1, m_gridDim.x - 1), std::min(gpt.y + 1, m_gridDim.y - 1), std::min(gpt.z + 1, m_gridDim.z - 1) };
		for (int32_t gz = gmn.z; gz <= gmx.z; ++gz)
		{
			for (int32_t gy = gmn.y; gy <= gmx.y; ++gy)
			{
				uint32_t offset = gridNodeIndex({ gmn.x, gy, gz });
				const Node* node = &m_nodes[offset];
				for (int32_t gx = gmn.x; gx <= gmx.x; ++gx)
				{
					int32_t size = node->size;
					fn(node->items, size);
					++node;
				}
			}
		}
	}

private:
	Aabb3 m_bounds;
	Scalar m_searchRadius = 0.0_simd;
	Grid m_gridDim;
	Node* m_nodes = nullptr;

	Grid calculateGrid(const Vector4& pt) const
	{
		Vector4 d = (pt - m_bounds.mn) / m_searchRadius;
		int32_t T_MATH_ALIGN16 e[4];
		d.storeIntegersAligned(e);
		return { e[0], e[1], e[2] };
	}

	uint32_t gridNodeIndex(const Grid& g) const
	{
		return g.x + g.y * m_gridDim.x + g.z * (m_gridDim.x * m_gridDim.y);
	}
};

	}
}
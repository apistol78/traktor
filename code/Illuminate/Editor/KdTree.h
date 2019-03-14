#pragma once

#include <limits>
#include <vector>
#include <algorithm>
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace illuminate
	{

template < typename ItemType >
struct DefaultPositionAccessor
{
	static const Vector4& get(const ItemType& item) { return item; }
};

template < typename ItemType, typename PositionAccessor = DefaultPositionAccessor< ItemType > >
class KdTree
{
public:
	const static int c_terminateIndex = ~0;

	KdTree()
	:	m_root(0)
	{
	}

	void build(const AlignedVector< ItemType >& items)
	{
		m_items = items;
		m_nodes.resize(items.size());

		for (size_t i = 0; i < items.size(); ++i)
		{
			m_nodes[i].partition = PositionAccessor::get(items[i]);
			m_nodes[i].item = int(i);
			m_nodes[i].left = c_terminateIndex;
			m_nodes[i].right = c_terminateIndex;
		}

		m_root = buildTraverse(0, int(items.size()), 0);
	}

	void insert(const ItemType& item)
	{
		Node nn =
		{
			PositionAccessor::get(item),
			int(m_items.size()),
			c_terminateIndex,
			c_terminateIndex
		};

		if (!m_nodes.empty())
		{
			int nodeIndex = m_root;
			for (int depth = 0;; ++depth)
			{
				int axis = depth % 3;
				Node& node = m_nodes[nodeIndex];
				if (node.partition[axis] < PositionAccessor::get(item)[axis])
				{
					if (node.left == c_terminateIndex)
					{
						node.left = int(m_nodes.size());
						break;
					}
					else
						nodeIndex = node.left;
				}
				else
				{
					if (node.right == c_terminateIndex)
					{
						node.right = int(m_nodes.size());
						break;
					}
					else
						nodeIndex = node.right;
				}
			}
		}

		m_items.push_back(item);
		m_nodes.push_back(nn);
	}

	size_t size() const
	{
		return m_items.size();
	}

	const ItemType& operator [] (size_t index) const
	{
		return m_items[index];
	}

	const ItemType* queryClosest(const Vector4& point, float maxDistance = std::numeric_limits< float >::max()) const
	{
		QueryResult result = queryClosestTraverse(point, m_root, 0, maxDistance);
		if (!result.node)
			return 0;
		return &m_items[result.node->item];
	}

	struct ClosestItem
	{
		float distance2;
		const ItemType* item;
	};

	size_t queryNClosest(const Vector4& point, AlignedVector< typename KdTree< ItemType >::ClosestItem >& out) const
	{
		for (size_t i = 0; i < out.size(); ++i)
		{
			out[i].distance2 = std::numeric_limits< float >::max();
			out[i].item = 0;
		}

		queryNClosestTraverse(point, m_root, 0, out);

		return out.size();
	}

	size_t queryWithinDistance(const Vector4& point, float distance, AlignedVector< ItemType >& out) const
	{
		queryWithinDistanceTraverse(point, m_root, 0, distance, out);
		return out.size();
	}

private:
	struct Node
	{
		Vector4 partition;
		int item;
		int left;
		int right;
	};

	AlignedVector< Node > m_nodes;
	AlignedVector< ItemType > m_items;
	int m_root;

	struct SortAxis
	{
		int m_axis;

		SortAxis(int axis)
		:	m_axis(axis)
		{
		}

		bool operator () (const Node& l, const Node& r) const
		{
			return bool(l.partition[m_axis] > r.partition[m_axis]);
		}
	};

	int buildTraverse(int start, int end, int depth)
	{
		std::sort(m_nodes.begin() + start, m_nodes.begin() + end, SortAxis(depth % 3));

		int pivot = (start + end) / 2;
		if (pivot > start)
			m_nodes[pivot].left = buildTraverse(start, pivot, depth + 1);
		if (pivot + 1 < end)
			m_nodes[pivot].right = buildTraverse(pivot + 1, end, depth + 1);

		return pivot;
	}

	struct QueryResult
	{
		float distance;
		const Node* node;
	};

	QueryResult queryClosestTraverse(const Vector4& point, int nodeIndex, int depth, float maxDistance2) const
	{
		QueryResult result = { maxDistance2 + 1.0f, 0 };
		if (nodeIndex != c_terminateIndex)
		{
			int axis = depth % 3;
			const Node& node = m_nodes[nodeIndex];
			
			float dist2 = dot3(node.partition, point);
			if (dist2 < result.distance)
			{
				result.distance = dist2;
				result.node = &node;
			}

			float split = node.partition[axis] - point[axis];
			float split2 = split * split;

			if (node.partition[axis] < point[axis])
			{
				QueryResult leftResult = queryClosestTraverse(point, node.left, depth + 1, result.distance);
				if (leftResult.distance < result.distance)
					result = leftResult;
				if (split2 < result.distance)
				{
					QueryResult rightResult = queryClosestTraverse(point, node.right, depth + 1, result.distance);
					if (rightResult.distance < result.distance)
						result = rightResult;
				}
			}
			else
			{
				QueryResult rightResult = queryClosestTraverse(point, node.right, depth + 1, result.distance);
				if (rightResult.distance < result.distance)
					result = rightResult;
				if (split2 < result.distance)
				{
					QueryResult leftResult = queryClosestTraverse(point, node.left, depth + 1, result.distance);
					if (leftResult.distance < result.distance)
						result = leftResult;
				}
			}
		}
		return result;
	}

	void queryNClosestTraverse(
		const Vector4& point,
		int nodeIndex,
		int depth,
		AlignedVector< typename KdTree< ItemType >::ClosestItem >& out
	) const
	{
		if (nodeIndex == c_terminateIndex)
			return;

		int axis = depth % 3;
		const Node& node = m_nodes[nodeIndex];

		float split = node.partition[axis] - point[axis];
		float split2 = split * split;

		if (node.partition[axis] < point[axis])
		{
			queryNClosestTraverse(point, node.left, depth + 1, out);

			float distance2 = dot3(point - node.partition, point - node.partition);
			if (distance2 < out[0].distance2)
			{
				out[0].distance2 = distance2;
				out[0].item = &m_items[node.item];

				for (size_t i = 1; i < out.size(); ++i)
				{
					if (out[i].distance2 > out[0].distance2)
						std::swap(out[i], out[0]);
				}
			}

			if (split2 < out[0].distance2)
				queryNClosestTraverse(point, node.right, depth + 1, out);
		}
		else
		{
			queryNClosestTraverse(point, node.right, depth + 1, out);

			float distance2 = dot3(point - node.partition, point - node.partition);
			if (distance2 < out[0].distance2)
			{
				out[0].distance2 = distance2;
				out[0].item = &m_items[node.item];

				for (size_t i = 1; i < out.size(); ++i)
				{
					if (out[i].distance2 > out[0].distance2)
						std::swap(out[i], out[0]);
				}
			}

			if (split2 < out[0].distance2)
				queryNClosestTraverse(point, node.left, depth + 1, out);
		}
	}

	void queryWithinDistanceTraverse(
		const Vector4& point,
		int nodeIndex,
		int depth,
		float distance,
		AlignedVector< ItemType >& out
	) const
	{
		if (nodeIndex == c_terminateIndex)
			return;
		
		int axis = depth % 3;
		const Node& node = m_nodes[nodeIndex];

		float split2 = dot3(node.partition - point, node.partition - point);
		if (split2 <= distance * distance)
			out.push_back(m_items[node.item]);

		if (node.partition[axis] < point[axis] + distance)
			queryWithinDistanceTraverse(point, node.left, depth + 1, distance, out);

		if (node.partition[axis] > point[axis] - distance)
			queryWithinDistanceTraverse(point, node.right, depth + 1, distance, out);
	}
};

	}
}

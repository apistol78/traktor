#include <limits>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/SahTree.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{
	namespace
	{

const int32_t c_maxDepth = 20;
const int32_t c_indicesCountThreshold = 10;

struct SplitCandidate
{
	Scalar position;
	int32_t countLeft;
	int32_t countRight;
};

	}

SahTree::SahTree()
:	m_root(0)
,	m_queryTag(0)
{
}

SahTree::~SahTree()
{
	for (AlignedVector< Node* >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
		Alloc::freeAlign(*i);
}

void SahTree::build(const AlignedVector< Winding3 >& polygons)
{
	// Create root node.
	m_root = allocNode();

	// Build list of transformed triangles from each shape.
	m_polygons.reserve(polygons.size());
	for (AlignedVector< Winding3 >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		for (AlignedVector< Vector4 >::const_iterator j = i->points.begin(); j != i->points.end(); ++j)
			m_root->aabb.contain(*j);

		m_root->indices.push_back(int32_t(m_polygons.size()));
		m_polygons.push_back(*i);
	}

	// Build spatial tree.
	buildNode(m_root, 0);

	// Query mask array.
	m_query.resize(m_polygons.size(), 0);
}

bool SahTree::queryClosestIntersection(const Vector4& origin, const Vector4& direction, QueryResult& outResult) const
{
	// \fixme
	return false;
}

bool SahTree::queryAnyIntersection(const Vector4& origin, const Vector4& direction, float maxDistance) const
{
	#define IS_LEAF(node) ((node)->leftChild == 0)

	const Scalar F(1e-3f);
	const Scalar md(maxDistance);
	Scalar nearT, farT;
	Scalar T;

	if (!m_root->aabb.intersectRay(origin, direction, nearT, farT) || farT < 0.0f)
		return false;

	if (nearT < 0.0f)
		nearT = Scalar(0.0f);

	m_queryTag++;

	m_stack.resize(0);
	m_stack.push_back(Stack(m_root, nearT, farT));

	while (!m_stack.empty())
	{
		Node* N = m_stack.back().node;
		nearT = m_stack.back().nearT;
		farT = m_stack.back().farT;

		m_stack.pop_back();

		if (IS_LEAF(N))
		{
			for (std::vector< int32_t >::iterator i = N->indices.begin(); i != N->indices.end(); ++i)
			{
				if (m_query[*i] == m_queryTag)
					continue;

				const Winding3& polygon = m_polygons[*i];
				if (polygon.rayIntersection(origin, direction, T))
				{
					if (T >= -F && (md <= F || T <= md))
						return true;
				}

				m_query[*i] = m_queryTag;
			}
		}
		else
		{
			Vector4 O = origin + direction * nearT;
			Scalar e = O[N->axis];

			if (e <= N->split + F)
			{
				T = nearT + Scalar(N->split - e) / direction[N->axis];
				if (T >= nearT && T <= farT)
				{
					m_stack.push_back(Stack(N->leftChild, nearT, T));
					m_stack.push_back(Stack(N->rightChild, T, farT));
				}
				else
					m_stack.push_back(Stack(N->leftChild, nearT, farT));
			}

			if (e >= N->split - F)
			{
				T = nearT + Scalar(N->split - e) / direction[N->axis];
				if (T >= nearT  && T <= farT)
				{
					m_stack.push_back(Stack(N->rightChild, nearT, T));
					m_stack.push_back(Stack(N->leftChild, T, farT));
				}
				else
					m_stack.push_back(Stack(N->rightChild, nearT, farT));
			}
		}
	}

	return false;
}

void SahTree::buildNode(Node* node, int32_t depth)
{
	node->axis = majorAxis3(node->aabb.getExtent());

	// Don't recurse too deep.
	if (depth > c_maxDepth || node->indices.size() <= c_indicesCountThreshold)
		return;

	// Collect possible split positions.
	AlignedVector< SplitCandidate > splitCandidates;
	AlignedVector< std::pair< Scalar, Scalar > > spatialRanges;
	for (std::vector< int32_t >::const_iterator i = node->indices.begin(); i != node->indices.end(); ++i)
	{
		const Winding3& polygon = m_polygons[*i];
		
		std::pair< Scalar, Scalar > range(
			Scalar(std::numeric_limits< float >::max()),
			Scalar(-std::numeric_limits< float >::max())
		);
		for (size_t j = 0; j < polygon.points.size(); ++j)
		{
			Scalar e = polygon.points[j][node->axis];
			
			if (
				e >= node->aabb.mn[node->axis] &&
				e <= node->aabb.mx[node->axis]
			)
			{
				SplitCandidate candidate = { e, 0, 0 };
				splitCandidates.push_back(candidate);
			}

			range.first = min(range.first, e);
			range.second = max(range.second, e);
		}

		spatialRanges.push_back(range);
	}

	// Determine left- and right count for each split position.
	for (AlignedVector< SplitCandidate >::iterator i = splitCandidates.begin(); i != splitCandidates.end(); ++i)
	{
		for (std::vector< int32_t >::iterator j = node->indices.begin(); j != node->indices.end(); ++j)
		{
			const std::pair< Scalar, Scalar >& range = spatialRanges[std::distance(node->indices.begin(), j)];
			if (range.first <= i->position)
				i->countLeft++;
			if (range.second >= i->position)
				i->countRight++;
		}
	}

	// Determine optimal split position.
	Vector4 extent = node->aabb.getExtent(); 
	Scalar surfaceArea = dot3(extent.shuffle< 0, 0, 1, 3 >(), extent.shuffle< 1, 2, 2, 3 >()) * Scalar(2.0f);

	Scalar leafCost(node->indices.size());
	Scalar lowestCost(std::numeric_limits< float >::max());
	SplitCandidate* bestCandidate = 0;
	Aabb bestLeftAabb;
	Aabb bestRightAabb;

	for (AlignedVector< SplitCandidate >::iterator i = splitCandidates.begin(); i != splitCandidates.end(); ++i)
	{
		Aabb leftAabb = node->aabb;
		leftAabb.mx.set(node->axis, i->position);

		Aabb rightAabb = node->aabb;
		rightAabb.mn.set(node->axis, i->position);

		Vector4 leftAabbExtent = leftAabb.getExtent();
		Scalar surfaceAreaLeft = dot3(leftAabbExtent.shuffle< 0, 0, 1, 3 >(), leftAabbExtent.shuffle< 1, 2, 2, 3 >()) * Scalar(2.0f);

		Vector4 rightAabbExtent = rightAabb.getExtent();
		Scalar surfaceAreaRight = dot3(rightAabbExtent.shuffle< 0, 0, 1, 3 >(), rightAabbExtent.shuffle< 1, 2, 2, 3 >()) * Scalar(2.0f);

		Scalar splitCost =
			Scalar(0.3f) +
			surfaceAreaLeft * Scalar(i->countLeft) / surfaceArea +
			surfaceAreaRight * Scalar(i->countRight) / surfaceArea;

		if (splitCost < lowestCost)
		{
			lowestCost = splitCost;
			bestCandidate = &(*i);
			bestLeftAabb = leftAabb;
			bestRightAabb = rightAabb;
		}
	}

	if (lowestCost > leafCost)
		return;

	// Split triangles into left and right sets.
	std::vector< int32_t > leftIndices, rightIndices;
	for (std::vector< int32_t >::iterator i = node->indices.begin(); i != node->indices.end(); ++i)
	{
		const std::pair< Scalar, Scalar >& range = spatialRanges[std::distance(node->indices.begin(), i)];
		if (range.first <= bestCandidate->position)
			leftIndices.push_back(*i);
		if (range.second >= bestCandidate->position)
			rightIndices.push_back(*i);
	}
	T_ASSERT (bestCandidate->countLeft == leftIndices.size());
	T_ASSERT (bestCandidate->countRight == rightIndices.size());

	// Create child nodes.
	node->split = bestCandidate->position;

	node->leftChild = allocNode();
	node->leftChild->aabb = bestLeftAabb;
	node->leftChild->indices = leftIndices;
	node->leftChild->leftChild = 0;
	node->leftChild->rightChild = 0;

	node->rightChild = allocNode();
	node->rightChild->aabb = bestRightAabb;
	node->rightChild->indices = rightIndices;
	node->rightChild->leftChild = 0;
	node->rightChild->rightChild = 0;

	buildNode(node->leftChild, depth + 1);
	buildNode(node->rightChild, depth + 1);
}

SahTree::Node* SahTree::allocNode()
{
	void* ptr = Alloc::acquireAlign(sizeof(Node), 16, T_FILE_LINE);
	if (ptr)
	{
		Node* node = new (ptr) Node();
		m_nodes.push_back(node);
		return node;
	}
	else
		return 0;
}

}

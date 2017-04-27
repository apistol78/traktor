/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	float position;
	int32_t countLeft;
	int32_t countRight;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.SahTree", SahTree, Object)

SahTree::SahTree()
:	m_root(0)
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
	m_polygons.resize(polygons.size());
	m_projected.resize(polygons.size());
	m_projectedU.resize(polygons.size());
	m_projectedV.resize(polygons.size());
	m_planes.resize(polygons.size());

	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		const Winding3::points_t& points = polygons[i].getPoints();
		for (Winding3::points_t::const_iterator j = points.begin(); j != points.end(); ++j)
			m_root->aabb.contain(*j);

		m_root->indices.push_back(int32_t(i));
		
		m_polygons[i] = polygons[i];
		m_polygons[i].getProjection(m_projected[i], m_projectedU[i], m_projectedV[i]);
		m_polygons[i].getPlane(m_planes[i]);
	}

	// Build spatial tree.
	buildNode(m_root, 0);
}

bool SahTree::queryClosestIntersection(const Vector4& origin, const Vector4& direction, int32_t ignore, QueryResult& outResult, QueryCache& inoutCache) const
{
	#define IS_LEAF(node) ((node)->leftChild == 0)

	const Scalar F(1e-3f);
	bool result = false;
	Scalar nearT, farT;
	Scalar T;
	Vector4 p;

	outResult.index = -1;
	outResult.distance = Scalar(std::numeric_limits< float >::max());

	if (!m_root->aabb.intersectRay(origin, direction, nearT, farT) || farT < 0.0f)
		return false;

	if (nearT < 0.0f)
		nearT = Scalar(0.0f);

	BitVector& tags = inoutCache.tags;
	tags.assign(m_polygons.size(), false);

	if (ignore >= 0)
		tags.set(ignore);

	AlignedVector< QueryStack >& stack = inoutCache.stack;
	stack.reserve(64);
	stack.resize(0);
	stack.push_back(QueryStack(m_root, nearT, farT));

	while (!stack.empty())
	{
		Node* N = stack.back().node;
		nearT = stack.back().nearT;
		farT = stack.back().farT;

		stack.pop_back();

		if (IS_LEAF(N))
		{
			for (std::vector< int32_t >::const_iterator i = N->indices.begin(); i != N->indices.end(); ++i)
			{
				if (tags[*i])
					continue;

				const Plane& plane = m_planes[*i];
				if (plane.rayIntersection(origin, direction, T, &p) && T > 0.0f && T <= outResult.distance)
				{
					Vector2 pnt(
						dot3(m_projectedU[*i], p),
						dot3(m_projectedV[*i], p)
					);
					if (m_projected[*i].inside(pnt))
					{
						outResult.index = *i;
						outResult.distance = T;
						outResult.position = origin + direction * T;
						outResult.normal = plane.normal();
						result = true;
					}
				}

				tags.set(*i);
			}
		}
		else
		{
			Vector4 O = origin + direction * nearT;
			Scalar e = O[N->axis];

			T = nearT + (N->split - e) / direction[N->axis];

			if (e <= N->split)
			{
				if (T >= nearT && T <= farT)
				{
					stack.push_back(QueryStack(N->leftChild, nearT, T));
					stack.push_back(QueryStack(N->rightChild, T, farT));
				}
				else
					stack.push_back(QueryStack(N->leftChild, nearT, farT));
			}
			else
			{
				if (T >= nearT  && T <= farT)
				{
					stack.push_back(QueryStack(N->rightChild, nearT, T));
					stack.push_back(QueryStack(N->leftChild, T, farT));
				}
				else
					stack.push_back(QueryStack(N->rightChild, nearT, farT));
			}
		}
	}

	return result;
}

bool SahTree::queryAnyIntersection(const Vector4& origin, const Vector4& direction, float maxDistance, int32_t ignore, QueryCache& inoutCache) const
{
	#define IS_LEAF(node) ((node)->leftChild == 0)

	const Scalar F(1e-3f);
	const Scalar md(maxDistance);
	Scalar nearT, farT;
	Scalar T;
	Vector4 p;

	if (!m_root->aabb.intersectRay(origin, direction, nearT, farT) || farT < 0.0f)
		return false;

	if (nearT < 0.0f)
		nearT = Scalar(0.0f);

	BitVector& tags = inoutCache.tags;
	tags.assign(m_polygons.size(), false);

	if (ignore >= 0)
		tags.set(ignore);

	AlignedVector< QueryStack >& stack = inoutCache.stack;
	stack.reserve(64);
	stack.resize(0);
	stack.push_back(QueryStack(m_root, nearT, farT));

	while (!stack.empty())
	{
		Node* N = stack.back().node;
		nearT = stack.back().nearT;
		farT = stack.back().farT;

		stack.pop_back();

		if (IS_LEAF(N))
		{
			for (std::vector< int32_t >::const_iterator i = N->indices.begin(); i != N->indices.end(); ++i)
			{
				if (tags[*i])
					continue;

				const Plane& plane = m_planes[*i];
				if (plane.rayIntersection(origin, direction, T, &p) && T > 0.0f && T < md)
				{
					Vector2 pnt(
						dot3(m_projectedU[*i], p),
						dot3(m_projectedV[*i], p)
					);
					if (m_projected[*i].inside(pnt))
						return true;
				}

				tags.set(*i);
			}
		}
		else
		{
			Vector4 O = origin + direction * nearT;
			Scalar e = O[N->axis];

			T = nearT + (N->split - e) / direction[N->axis];

			if (e <= N->split)
			{
				if (T >= nearT && T <= farT)
				{
					stack.push_back(QueryStack(N->leftChild, nearT, T));
					stack.push_back(QueryStack(N->rightChild, T, farT));
				}
				else
					stack.push_back(QueryStack(N->leftChild, nearT, farT));
			}
			else
			{
				if (T >= nearT  && T <= farT)
				{
					stack.push_back(QueryStack(N->rightChild, nearT, T));
					stack.push_back(QueryStack(N->leftChild, T, farT));
				}
				else
					stack.push_back(QueryStack(N->rightChild, nearT, farT));
			}
		}
	}

	return false;
}

bool SahTree::checkPoint(int32_t index, const Vector4& position) const
{
	Vector2 pnt(
		dot3(m_projectedU[index], position),
		dot3(m_projectedV[index], position)
	);
	return m_projected[index].inside(pnt);
}

void SahTree::buildNode(Node* node, int32_t depth)
{
	node->axis = majorAxis3(node->aabb.getExtent());

	// Don't recurse too deep.
	if (depth > c_maxDepth || node->indices.size() <= c_indicesCountThreshold)
		return;

	// Collect possible split positions.
	AlignedVector< SplitCandidate > splitCandidates;
	AlignedVector< std::pair< float, float > > spatialRanges;

	spatialRanges.reserve(node->indices.size());
	for (std::vector< int32_t >::const_iterator i = node->indices.begin(); i != node->indices.end(); ++i)
	{
		const Winding3& polygon = m_polygons[*i];
		const Winding3::points_t& points = polygon.getPoints();
		
		std::pair< float, float > range(
			std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max()
		);
		for (size_t j = 0; j < points.size(); ++j)
		{
			float e = points[j][node->axis];
			
			if (
				e >= node->aabb.mn[node->axis] + FUZZY_EPSILON &&
				e <= node->aabb.mx[node->axis] - FUZZY_EPSILON
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
		for (uint32_t j = 0; j < node->indices.size(); ++j)
		{
			const std::pair< float, float >& range = spatialRanges[j];
			if (range.first <= i->position)
				i->countLeft++;
			if (range.second >= i->position)
				i->countRight++;
		}
	}

	// Determine optimal split position.
	Vector4 extent = node->aabb.getExtent(); 
	float surfaceArea = dot3(extent.shuffle< 0, 0, 1, 3 >(), extent.shuffle< 1, 2, 2, 3 >()) * Scalar(2.0f);

	float leafCost(float(node->indices.size()));
	float lowestCost(std::numeric_limits< float >::max());
	SplitCandidate* bestCandidate = 0;
	Aabb3 bestLeftAabb;
	Aabb3 bestRightAabb;

	for (AlignedVector< SplitCandidate >::iterator i = splitCandidates.begin(); i != splitCandidates.end(); ++i)
	{
		Aabb3 leftAabb = node->aabb;
		leftAabb.mx.set(node->axis, Scalar(i->position));

		Aabb3 rightAabb = node->aabb;
		rightAabb.mn.set(node->axis, Scalar(i->position));

		Vector4 leftAabbExtent = leftAabb.getExtent();
		float surfaceAreaLeft = dot3(leftAabbExtent.shuffle< 0, 0, 1, 3 >(), leftAabbExtent.shuffle< 1, 2, 2, 3 >()) * Scalar(2.0f);

		Vector4 rightAabbExtent = rightAabb.getExtent();
		float surfaceAreaRight = dot3(rightAabbExtent.shuffle< 0, 0, 1, 3 >(), rightAabbExtent.shuffle< 1, 2, 2, 3 >()) * Scalar(2.0f);

		float splitCost =
			float(0.3f) +
			surfaceAreaLeft * float(i->countLeft) / surfaceArea +
			surfaceAreaRight * float(i->countRight) / surfaceArea;

		if (splitCost < lowestCost)
		{
			lowestCost = splitCost;
			bestCandidate = &(*i);
			bestLeftAabb = leftAabb;
			bestRightAabb = rightAabb;
		}
	}

	if (!bestCandidate || lowestCost > leafCost)
		return;

	// Split triangles into left and right sets.
	std::vector< int32_t > leftIndices, rightIndices;
	leftIndices.reserve(bestCandidate->countLeft);
	rightIndices.reserve(bestCandidate->countRight);
	for (std::vector< int32_t >::iterator i = node->indices.begin(); i != node->indices.end(); ++i)
	{
		const std::pair< float, float >& range = spatialRanges[std::distance(node->indices.begin(), i)];
		if (range.first <= bestCandidate->position)
			leftIndices.push_back(*i);
		if (range.second >= bestCandidate->position)
			rightIndices.push_back(*i);
	}
	T_ASSERT (bestCandidate->countLeft == leftIndices.size());
	T_ASSERT (bestCandidate->countRight == rightIndices.size());

	// Create child nodes.
	node->split = Scalar(bestCandidate->position);

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

	// Release memory before recursing.
	leftIndices.clear();
	rightIndices.clear();
	splitCandidates.clear();
	spatialRanges.clear();

	// Recursively build branches.
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

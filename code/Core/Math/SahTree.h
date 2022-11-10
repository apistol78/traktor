/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/BitVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Winding2.h"
#include "Core/Math/Winding3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! SAH tree.
 *
 * Simple kD-tree with "surface area heuristic"
 * split determination.
 *
 * \ingroup Core
 */
class T_DLLCLASS SahTree : public Object
{
	T_RTTI_CLASS;

	struct Node;

public:
	struct QueryResult
	{
		int32_t index;
		Scalar distance;
		Vector4 position;
		Vector4 normal;

		QueryResult()
		:	index(-1)
		,	distance(0.0f)
		{
		}
	};

	struct QueryStack
	{
		Node* node;
		Scalar nearT;
		Scalar farT;

		QueryStack()
		:	node(0)
		,	nearT(0.0f)
		,	farT(1e9f)
		{
		}

		QueryStack(Node* _node, float _nearT, float _farT)
		:	node(_node)
		,	nearT(_nearT)
		,	farT(_farT)
		{
		}
	};

	struct QueryCache
	{
		BitVector tags;
		AlignedVector< QueryStack > stack;
	};

	SahTree();

	virtual ~SahTree();

	/*! Build tree from a set of polygons.
	 *
	 * \param polygons Polygon set.
	 * \return True if built successfully.
	 */
	void build(const AlignedVector< Winding3 >& polygons);

	/*! Query for closest intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param maxDistance Intersection must occur prior to this distance from origin, 0 distance is infinite.
	 * \param ignore Ignore intersection with winding.
	 * \param outResult Intersection result.
	 * \return True if any intersection found.
	 */
	bool queryClosestIntersection(const Vector4& origin, const Vector4& direction, float maxDistance, int32_t ignore, QueryResult& outResult, QueryCache& inoutCache) const;

	/*! Query for closest intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param ignore Ignore intersection with winding.
	 * \param outResult Intersection result.
	 * \return True if any intersection found.
	 */
	bool queryClosestIntersection(const Vector4& origin, const Vector4& direction, int32_t ignore, QueryResult& outResult, QueryCache& inoutCache) const {
		return queryClosestIntersection(origin, direction, 0.0f, -1, outResult, inoutCache);
	}

	/*! Query for closest intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param outResult Intersection result.
	 * \return True if any intersection found.
	 */
	bool queryClosestIntersection(const Vector4& origin, const Vector4& direction, QueryResult& outResult, QueryCache& inoutCache) const {
		return queryClosestIntersection(origin, direction, -1, outResult, inoutCache);
	}

	/*! Query for any intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param maxDistance Intersection must occur prior to this distance from origin, 0 distance is infinite.
	 * \param ignore Ignore intersection with winding.
	 * \return True if any intersection found.
	 */
	bool queryAnyIntersection(const Vector4& origin, const Vector4& direction, float maxDistance, int32_t ignore, QueryCache& inoutCache) const;

	/*! Query for any intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param maxDistance Intersection must occur prior to this distance from origin, 0 distance is infinite.
	 * \return True if any intersection found.
	 */
	bool queryAnyIntersection(const Vector4& origin, const Vector4& direction, float maxDistance, QueryCache& inoutCache) const {
		return queryAnyIntersection(origin, direction, maxDistance, -1, inoutCache);
	}

	/*! Check if point is within winding.
	 *
	 * \param index Index of winding.
	 * \param position Position of point.
	 * \return True if point is within winding.
	 */
	bool checkPoint(int32_t index, const Vector4& position) const;

	/*! Get polygons. */
	const AlignedVector< Winding3 >& getPolygons() const { return m_polygons; }

	/*! Get bounding box. */
	const Aabb3& getBoundingBox() const { return m_root->aabb; }

private:
	struct Node
	{
		Aabb3 aabb;
		AlignedVector< int32_t > indices;
		int32_t axis;
		Scalar split;
		Node* leftChild;
		Node* rightChild;

		Node()
		:	axis(0)
		,	split(0.0f)
		,	leftChild(0)
		,	rightChild(0)
		{
		}
	};

	Node* m_root;
	AlignedVector< Winding3 > m_polygons;
	AlignedVector< Winding2 > m_projected;
	AlignedVector< Vector4 > m_projectedU;
	AlignedVector< Vector4 > m_projectedV;
	AlignedVector< Plane > m_planes;
	AlignedVector< Node* > m_nodes;

	void buildNode(Node* node, int32_t depth);

	Node* allocNode();
};

}


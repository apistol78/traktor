#ifndef traktor_mesh_SahTree_H
#define traktor_mesh_SahTree_H

#include <vector>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Winding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief SAH tree.
 *
 * Simple kD-tree with "surface area heuristic"
 * split determination.
 *
 * \ingroup Core
 */
class T_DLLCLASS SahTree
{
public:
	struct QueryResult { /* \fixme */ };

	SahTree();

	virtual ~SahTree();

	/*! \brief Build tree from a set of polygons.
	 *
	 * \param polygons Polygon set.
	 * \return True if built successfully.
	 */
	void build(const AlignedVector< Winding >& polygons);

	/*! \brief Query for closest intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param outResult Intersection result.
	 * \return True if any intersection found.
	 */
	bool queryClosestIntersection(const Vector4& origin, const Vector4& direction, QueryResult& outResult) const;

	/*! \brief Query for any intersection.
	 *
	 * \param origin Ray origin.
	 * \param direction Ray direction.
	 * \param maxDistance Intersection must occur prior to this distance from origin, 0 distance is infinite.
	 * \return True if any intersection found.
	 */
	bool queryAnyIntersection(const Vector4& origin, const Vector4& direction, float maxDistance) const;

private:
	struct Node
	{
		Aabb aabb;
		std::vector< int32_t > indices;
		int32_t axis;
		float split;
		Node* leftChild;
		Node* rightChild;
	};

	struct Stack
	{
		Node* node;
		Scalar nearT;
		Scalar farT;

		Stack()
		:	node(0)
		,	nearT(0.0f)
		,	farT(1e9f)
		{
		}

		Stack(Node* _node, const Scalar& _nearT, const Scalar& _farT)
		:	node(_node)
		,	nearT(_nearT)
		,	farT(_farT)
		{
		}
	};

	Node* m_root;
	AlignedVector< Winding > m_polygons;
	AlignedVector< Node* > m_nodes;
	mutable AlignedVector< Stack > m_stack;
	mutable std::vector< int32_t > m_query;
	mutable int32_t m_queryTag;

	void buildNode(Node* node, int32_t depth);

	Node* allocNode();
};

}

#endif	// traktor_mesh_SahTree_H

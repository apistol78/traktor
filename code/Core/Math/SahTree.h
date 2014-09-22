#ifndef traktor_mesh_SahTree_H
#define traktor_mesh_SahTree_H

#include <vector>
#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
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

/*! \brief SAH tree.
 *
 * Simple kD-tree with "surface area heuristic"
 * split determination.
 *
 * \ingroup Core
 */
class T_DLLCLASS SahTree : public Object
{
	T_RTTI_CLASS;

public:
	struct QueryResult
	{
		int32_t index;
		float distance;
		Vector4 position;
	};

	SahTree();

	virtual ~SahTree();

	/*! \brief Build tree from a set of polygons.
	 *
	 * \param polygons Polygon set.
	 * \return True if built successfully.
	 */
	void build(const AlignedVector< Winding3 >& polygons);

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

	/*! \brief Get polygons. */
	const AlignedVector< Winding3 >& getPolygons() const { return m_polygons; }

	/*! \brief Get bounding box. */
	const Aabb3& getBoundingBox() const { return m_root->aabb; }

private:
	struct Node
	{
		Aabb3 aabb;
		std::vector< int32_t > indices;
		int32_t axis;
		float split;
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

	struct Stack
	{
		Node* node;
		float nearT;
		float farT;

		Stack()
		:	node(0)
		,	nearT(0.0f)
		,	farT(1e9f)
		{
		}

		Stack(Node* _node, float _nearT, float _farT)
		:	node(_node)
		,	nearT(_nearT)
		,	farT(_farT)
		{
		}
	};

	Node* m_root;
	AlignedVector< Winding3 > m_polygons;
	AlignedVector< Node* > m_nodes;

	void buildNode(Node* node, int32_t depth);

	Node* allocNode();
};

}

#endif	// traktor_mesh_SahTree_H

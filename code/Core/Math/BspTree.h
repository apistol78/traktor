#pragma once

#include <functional>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
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

/*! Binary space partitioning tree.
 * \ingroup Core
 */
class T_DLLCLASS BspTree
{
public:
	enum ClipMode
	{
		CmFront = 1,
		CmBack = 2,
		CmBoth = (CmFront | CmBack)
	};

	BspTree();

	explicit BspTree(const AlignedVector< Winding3 >& polygons);

	/*! Build BSP from a set of polygons.
	 *
	 * \param polygons Polygon set.
	 * \return True if built successfully.
	 */
	bool build(const AlignedVector< Winding3 >& polygons);

	/*! Check if point is inside "solid" space.
	 *
	 * \return True if point inside "solid" space.
	 */
	bool inside(const Vector4& pt) const;

	/*! Check if all points of a winding is inside "solid" space.
	 *
	 * \return True if all points inside "solid" space.
	 */
	bool inside(const Winding3& w) const;

	/*! Clip windings to BSP.
	 */
	void clip(const Winding3& w, const std::function< void(const Winding3& w, uint32_t cl, bool splitted) >& visitor) const;

	/*! Clip polygon to BSP.
	 *
	 * \param polygon Polygon
	 * \param outClipped Clipped polygons.
	 */
	template < typename PolygonType >
	void clip(const PolygonType& polygon, uint32_t mode, AlignedVector< PolygonType >& outClipped) const
	{
		clip< PolygonType >(0, polygon, mode, outClipped);
	}

private:
	struct Node
	{
		Plane plane;
		int32_t front;
		int32_t back;

		Node()
		:	front(-1)
		,	back(-1)
		{
		}
	};

	AlignedVector< Node > m_nodes;

	int32_t internalBuild(AlignedVector< Winding3 >& polygons);

	bool inside(int32_t node, const Vector4& pt) const;

	bool inside(int32_t node, const Winding3& w) const;

	void clip(int32_t node, const Winding3& w, const Plane& wp, bool splitted, const std::function< void(const Winding3& w, uint32_t cl, bool splitted) >& visitor) const;

	template < typename PolygonType >
	void clip(int32_t node, const PolygonType& polygon, uint32_t mode, AlignedVector< PolygonType >& outClipped) const
	{
		const Node& n = m_nodes[node];
		Winding3 w = polygon.winding();

		int cf = w.classify(n.plane);
		if (cf == Winding3::CfCoplanar)
		{
			Plane polygonPlane;
			if (w.getPlane(polygonPlane))
				cf = dot3(n.plane.normal(), polygonPlane.normal()) >= 0.0f ? Winding3::CfFront : Winding3::CfBack;
			else
				cf = Winding3::CfFront;
		}

		if (cf == Winding3::CfFront)
		{
			if (n.front >= 0)
				clip(n.front, polygon, mode, outClipped);
			else if (polygon.valid() && (mode & CmFront) != 0)
				outClipped.push_back(polygon);
		}
		else if (cf == Winding3::CfBack)
		{
			if (n.back >= 0)
				clip(n.back, polygon, mode, outClipped);
			else if (polygon.valid() && (mode & CmBack) != 0)
				outClipped.push_back(polygon);
		}
		else if (cf == Winding3::CfSpan)
		{
			PolygonType f, b;
			polygon.split(n.plane, f, b);
			if (f.valid())
			{
				if (n.front >= 0)
					clip(n.front, f, mode, outClipped);
				else if ((mode & CmFront) != 0)
					outClipped.push_back(f);
			}
			if (b.valid())
			{
				if (n.back >= 0)
					clip(n.back, b, mode, outClipped);
				else if ((mode & CmBack) != 0)
					outClipped.push_back(b);
			}
		}
	}
};

}


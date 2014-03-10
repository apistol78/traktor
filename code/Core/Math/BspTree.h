#ifndef traktor_mesh_BspTree_H
#define traktor_mesh_BspTree_H

#include <vector>
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

/*! \brief Binary space partitioning tree.
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

	/*! \brief Build BSP from a set of polygons.
	 *
	 * \param polygons Polygon set.
	 * \return True if built successfully.
	 */
	bool build(const AlignedVector< Winding3 >& polygons);

	/*! \brief Check if point is inside "solid" space.
	 *
	 * \return True if point inside "solid" space.
	 */
	bool inside(const Vector4& pt) const;

	/*! \brief Check if all points of a winding is inside "solid" space.
	 *
	 * \return True if all points inside "solid" space.
	 */
	bool inside(const Winding3& w) const;

	/*! \brief Clip windings to BSP.
	 */
	template < typename VisitorType >
	void clip(const Winding3& w, VisitorType& visitor) const
	{
		T_ASSERT (m_root);
		clip_1< VisitorType >(m_root, w, false, visitor);
	}

	/*! \brief Clip polygon to BSP.
	 *
	 * \param polygon Polygon
	 * \param outClipped Clipped polygons.
	 */
	template < typename PolygonType >
	void clip(const PolygonType& polygon, uint32_t mode, AlignedVector< PolygonType >& outClipped) const
	{
		T_ASSERT (m_root);
		clip_2< PolygonType >(m_root, polygon, mode, outClipped);
	}

	/*! \brief Get all planes.
	 */
	const AlignedVector< Plane >& getPlanes() const { return m_planes; }

private:
	struct BspNode : public RefCountImpl< IRefCount >
	{
		uint32_t plane;		//!< \note The plane index is the same index into source winding set passed into build.
		Ref< BspNode > front;
		Ref< BspNode > back;
	};

	AlignedVector< Plane > m_planes;
	Ref< BspNode > m_root;

	Ref< BspNode > recursiveBuild(AlignedVector< Winding3 >& polygons, std::vector< uint32_t >& planes) const;

	bool inside(const BspNode* node, const Vector4& pt) const;

	bool inside(const BspNode* node, const Winding3& w) const;

	template < typename VisitorType >
	void clip_1(const BspNode* node, const Winding3& w, bool splitted, VisitorType& visitor) const
	{
		const Plane& p = m_planes[node->plane];

		int cf = w.classify(p);
		if (cf == Winding3::CfCoplanar)
		{
			Plane polygonPlane;
			if (w.getPlane(polygonPlane))
				cf = dot3(p.normal(), polygonPlane.normal()) >= 0.0f ? Winding3::CfFront : Winding3::CfBack;
			else
				cf = Winding3::CfFront;
		}

		if (cf == Winding3::CfFront)
		{
			if (node->front)
				clip_1(node->front, w, splitted, visitor);
			else
				visitor(node->plane, w, cf, splitted);
		}
		else if (cf == Winding3::CfBack)
		{
			if (node->back)
				clip_1(node->back, w, splitted, visitor);
			else
				visitor(node->plane, w, cf, splitted);
		}
		else if (cf == Winding3::CfSpan)
		{
			Winding3 f, b;
			w.split(p, f, b);
			if (!f.points.empty())
			{
				if (node->front)
					clip_1(node->front, f, true, visitor);
				else
					visitor(node->plane, f, Winding3::CfFront, true);
			}
			if (!b.points.empty())
			{
				if (node->back)
					clip_1(node->back, b, true, visitor);
				else
					visitor(node->plane, b, Winding3::CfBack, true);
			}
		}
	}

	template < typename PolygonType >
	void clip_2(const BspNode* node, const PolygonType& polygon, uint32_t mode, AlignedVector< PolygonType >& outClipped) const
	{
		Winding3 w = polygon.winding();
		const Plane& p = m_planes[node->plane];

		int cf = w.classify(p);
		if (cf == Winding3::CfCoplanar)
		{
			Plane polygonPlane;
			if (w.getPlane(polygonPlane))
				cf = dot3(p.normal(), polygonPlane.normal()) >= 0.0f ? Winding3::CfFront : Winding3::CfBack;
			else
				cf = Winding3::CfFront;
		}

		if (cf == Winding3::CfFront)
		{
			if (node->front)
				clip_2(node->front, polygon, mode, outClipped);
			else if (polygon.valid() && (mode & CmFront) != 0)
				outClipped.push_back(polygon);
		}
		else if (cf == Winding3::CfBack)
		{
			if (node->back)
				clip_2(node->back, polygon, mode, outClipped);
			else if (polygon.valid() && (mode & CmBack) != 0)
				outClipped.push_back(polygon);
		}
		else if (cf == Winding3::CfSpan)
		{
			PolygonType f, b;
			polygon.split(p, f, b);
			if (f.valid())
			{
				if (node->front)
					clip_2(node->front, f, mode, outClipped);
				else if ((mode & CmFront) != 0)
					outClipped.push_back(f);
			}
			if (b.valid())
			{
				if (node->back)
					clip_2(node->back, b, mode, outClipped);
				else if ((mode & CmBack) != 0)
					outClipped.push_back(b);
			}
		}
	}
};

}

#endif	// traktor_mesh_BspTree_H

#include "Core/Math/BspTree.h"
#include "Core/Math/Const.h"
#include "Core/Heap/New.h"
#include "Core/Log/Log.h"

namespace traktor
{

bool BspTree::build(const AlignedVector< Winding >& polygons)
{
	// Calculate polygon planes.
	AlignedVector< Plane > planes(polygons.size());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		if (!polygons[i].getPlane(planes[i]))
			return false;
	}

	// Recursively build nodes.
	m_root = recursiveBuild(polygons, planes);
	return bool(m_root != 0);
}

bool BspTree::inside(const Vector4& pt) const
{
	T_ASSERT (m_root);
	return inside(m_root, pt);
}

bool BspTree::inside(const Winding& w) const
{
	T_ASSERT (m_root);
	return inside(m_root, w);
}

BspTree::BspNode* BspTree::recursiveBuild(const AlignedVector< Winding >& polygons, const AlignedVector< Plane >& planes)
{
	Ref< BspNode > node = gc_new< BspNode >();
	node->plane = planes[0];

	AlignedVector< Winding > frontPolygons, backPolygons;
	AlignedVector< Plane > frontPlanes, backPlanes;

	for (size_t i = 1; i < polygons.size(); ++i)
	{
		int cf = polygons[i].classify(node->plane);
		if (cf == Winding::CfFront || cf == Winding::CfCoplanar)
		{
			frontPolygons.push_back(polygons[i]);
			frontPlanes.push_back(planes[i]);
		}
		else if (cf == Winding::CfBack)
		{
			backPolygons.push_back(polygons[i]);
			backPlanes.push_back(planes[i]);
		}
		else
		{
			T_ASSERT (cf == Winding::CfSpan);
			Winding f, b;
			
			polygons[i].split(node->plane, f, b);
			T_ASSERT (f.points.size() >= 3);
			T_ASSERT (b.points.size() >= 3);

			frontPolygons.push_back(f);
			frontPlanes.push_back(planes[i]);
			backPolygons.push_back(b);
			backPlanes.push_back(planes[i]);
		}
	}

	T_ASSERT (frontPolygons.size() == frontPlanes.size());
	T_ASSERT (backPolygons.size() == backPlanes.size());

	if (!frontPolygons.empty())
		node->front = recursiveBuild(frontPolygons, frontPlanes);
	if (!backPolygons.empty())
		node->back = recursiveBuild(backPolygons, backPlanes);

	return node;
}

bool BspTree::inside(const BspNode* node, const Vector4& pt) const
{
	float d = node->plane.distance(pt);
	if (d > FUZZY_EPSILON)
		return node->front ? inside(node->front, pt) : true;
	else if (d < -FUZZY_EPSILON)
		return node->back ? inside(node->back, pt) : false;
	else
	{
		if (node->front && inside(node->front, pt))
				return true;
		if (node->back && inside(node->back, pt))
				return true;
	}
	return false;
}

bool BspTree::inside(const BspNode* node, const Winding& w) const
{
	bool result = false;

	int cf = w.classify(node->plane);
	if (cf == Winding::CfFront || cf == Winding::CfCoplanar)
		result = node->front ? inside(node->front, w) : true;
	else if (cf == Winding::CfBack)
		result = node->back ? inside(node->back, w) : false;
	else
	{
		T_ASSERT (cf == Winding::CfSpan);
		Winding f, b;

		w.split(node->plane, f, b);
		T_ASSERT (!f.points.empty());
		T_ASSERT (!b.points.empty());

		if (node->front)
			result |= inside(node->front, f);
		if (node->back)
			result |= inside(node->back, b);
	}

	return result;
}

}

#include "Core/Log/Log.h"
#include "Core/Math/BspTree.h"
#include "Core/Math/Const.h"

namespace traktor
{

bool BspTree::build(const AlignedVector< Winding3 >& polygons)
{
	if (polygons.empty())
		return false;

	AlignedVector< Winding3 > mutablePolygons = polygons;
	std::vector< uint32_t > mutablePlanes(polygons.size());

	// Calculate polygon planes.
	m_planes.resize(mutablePolygons.size());
	for (uint32_t i = 0; i < uint32_t(mutablePolygons.size()); ++i)
	{
		if (!mutablePolygons[i].getPlane(m_planes[i]))
			return false;

		mutablePlanes[i] = i;
	}

	// Recursively build nodes.
	m_root = recursiveBuild(mutablePolygons, mutablePlanes);
	return bool(m_root != 0);
}

bool BspTree::inside(const Vector4& pt) const
{
	T_ASSERT (m_root);
	return inside(m_root, pt);
}

bool BspTree::inside(const Winding3& w) const
{
	T_ASSERT (m_root);
	return inside(m_root, w);
}

Ref< BspTree::BspNode > BspTree::recursiveBuild(AlignedVector< Winding3 >& polygons, std::vector< uint32_t >& planes) const
{
	Ref< BspNode > node = new BspNode();
	node->plane = planes[0];

	const Plane& p = m_planes[node->plane];

	AlignedVector< Winding3 > frontPolygons, backPolygons;
	std::vector< uint32_t > frontPlanes, backPlanes;

	for (size_t i = 1; i < polygons.size(); ++i)
	{
		int cf = polygons[i].classify(p);
		if (cf == Winding3::CfFront || cf == Winding3::CfCoplanar)
		{
			frontPolygons.push_back(polygons[i]);
			frontPlanes.push_back(planes[i]);
		}
		else if (cf == Winding3::CfBack)
		{
			backPolygons.push_back(polygons[i]);
			backPlanes.push_back(planes[i]);
		}
		else
		{
			T_ASSERT (cf == Winding3::CfSpan);
			Winding3 f, b;
			
			polygons[i].split(p, f, b);
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

	// Discard input windings here; not used any more and
	// we don't want an memory explosion.
	polygons.clear();
	planes.clear();

	if (!frontPolygons.empty())
		node->front = recursiveBuild(frontPolygons, frontPlanes);
	if (!backPolygons.empty())
		node->back = recursiveBuild(backPolygons, backPlanes);

	return node;
}

bool BspTree::inside(const BspNode* node, const Vector4& pt) const
{
	float d = m_planes[node->plane].distance(pt);
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

bool BspTree::inside(const BspNode* node, const Winding3& w) const
{
	bool result = false;

	int cf = w.classify(m_planes[node->plane]);
	if (cf == Winding3::CfFront || cf == Winding3::CfCoplanar)
		result = node->front ? inside(node->front, w) : true;
	else if (cf == Winding3::CfBack)
		result = node->back ? inside(node->back, w) : false;
	else
	{
		T_ASSERT (cf == Winding3::CfSpan);
		Winding3 f, b;

		w.split(m_planes[node->plane], f, b);
		T_ASSERT (!f.points.empty());
		T_ASSERT (!b.points.empty());

		if (node->front)
			result |= inside(node->front, f);
		if (node->back)
			result |= inside(node->back, b);
	}

	return result;
}

//void* BspTree::BspNode::operator new (size_t size)
//{
//	return getAllocator()->alloc(size, 16, "BspNode");
//}
//
//void BspTree::BspNode::operator delete (void* ptr)
//{
//	getAllocator()->free(ptr);
//}

}

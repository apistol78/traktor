#include "Core/Log/Log.h"
#include "Core/Math/BspTree.h"
#include "Core/Math/Const.h"

namespace traktor
{

BspTree::BspTree()
:	m_root(-1)
{
}

BspTree::BspTree(const AlignedVector< Winding3 >& polygons)
:	m_root(-1)
{
	build(polygons);
}

bool BspTree::build(const AlignedVector< Winding3 >& polygons)
{
	if (polygons.empty())
		return false;

	AlignedVector< Winding3 > mutablePolygons = polygons;
	AlignedVector< uint32_t > mutablePlanes(polygons.size());

	// Calculate polygon planes.
	m_planes.resize(mutablePolygons.size());
	for (uint32_t i = 0; i < (uint32_t)mutablePolygons.size(); ++i)
	{
		if (!mutablePolygons[i].getPlane(m_planes[i]))
			return false;

		mutablePlanes[i] = i;
	}

	// Initially reserve equal amount of nodes as windings, probably more but never less.
	m_nodes.reserve(polygons.size());

	// Recursively build nodes.
	m_root = build(mutablePolygons, mutablePlanes);
	return (bool)(m_root >= 0);
}

bool BspTree::inside(const Vector4& pt) const
{
	T_ASSERT(m_root);
	return inside(m_root, pt);
}

bool BspTree::inside(const Winding3& w) const
{
	T_ASSERT(m_root);
	return inside(m_root, w);
}

void BspTree::clip(const Winding3& w, const std::function< void(uint32_t index, const Winding3& w, uint32_t cl, bool splitted) >& visitor) const
{
	T_ASSERT(m_root);
	clip(m_root, w, false, visitor);
}

int32_t BspTree::build(AlignedVector< Winding3 >& polygons, AlignedVector< uint32_t >& planes)
{
	int32_t node = (int32_t)m_nodes.size();
	Node& n = m_nodes.push_back();

	n.plane = planes[0];

	const Plane& p = m_planes[n.plane];

	AlignedVector< Winding3 > frontPolygons, backPolygons;
	AlignedVector< uint32_t > frontPlanes, backPlanes;

	for (size_t i = 1; i < polygons.size(); ++i)
	{
		int32_t cf = polygons[i].classify(p);
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
			T_ASSERT(cf == Winding3::CfSpan);
			Winding3 f, b;

			polygons[i].split(p, f, b);

			T_FATAL_ASSERT(f.size() >= 3);
			T_FATAL_ASSERT(f.area() >= FUZZY_EPSILON);
			T_FATAL_ASSERT(b.size() >= 3);
			T_FATAL_ASSERT(b.area() >= FUZZY_EPSILON);

			frontPolygons.push_back(f);
			frontPlanes.push_back(planes[i]);
			backPolygons.push_back(b);
			backPlanes.push_back(planes[i]);
		}
	}

	T_ASSERT(frontPolygons.size() == frontPlanes.size());
	T_ASSERT(backPolygons.size() == backPlanes.size());

	// Discard input windings here; not used any more and
	// we don't want an memory explosion.
	polygons.clear();
	planes.clear();

	if (!frontPolygons.empty())
		n.front = build(frontPolygons, frontPlanes);
	if (!backPolygons.empty())
		n.back = build(backPolygons, backPlanes);

	return node;
}

bool BspTree::inside(int32_t node, const Vector4& pt) const
{
	const Node& n = m_nodes[node];
	float d = m_planes[n.plane].distance(pt);
	if (d > FUZZY_EPSILON)
		return (n.front >= 0) ? inside(n.front, pt) : true;
	else if (d < -FUZZY_EPSILON)
		return (n.back >= 0) ? inside(n.back, pt) : false;
	else
	{
		if (n.front >= 0 && inside(n.front, pt))
				return true;
		if (n.back >= 0 && inside(n.back, pt))
				return true;
	}
	return false;
}

bool BspTree::inside(int32_t node, const Winding3& w) const
{
	const Node& n = m_nodes[node];
	bool result = false;

	int cf = w.classify(m_planes[n.plane]);
	if (cf == Winding3::CfFront || cf == Winding3::CfCoplanar)
		result = (n.front >= 0) ? inside(n.front, w) : true;
	else if (cf == Winding3::CfBack)
		result = (n.back >= 0) ? inside(n.back, w) : false;
	else
	{
		T_ASSERT(cf == Winding3::CfSpan);
		Winding3 f, b;

		w.split(m_planes[n.plane], f, b);
		T_ASSERT(!f.empty());
		T_ASSERT(!b.empty());

		if (n.front >= 0)
			result |= inside(n.front, f);
		if (n.back >= 0)
			result |= inside(n.back, b);
	}

	return result;
}

void BspTree::clip(int32_t node, const Winding3& w, bool splitted, const std::function< void(uint32_t index, const Winding3& w, uint32_t cl, bool splitted) >& visitor) const
{
	const Node& n = m_nodes[node];
	const Plane& p = m_planes[n.plane];

	int32_t cf = w.classify(p);
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
		if (n.front >= 0)
			clip(n.front, w, splitted, visitor);
		else
			visitor(n.plane, w, cf, splitted);
	}
	else if (cf == Winding3::CfBack)
	{
		if (n.back >= 0)
			clip(n.back, w, splitted, visitor);
		else
			visitor(n.plane, w, cf, splitted);
	}
	else if (cf == Winding3::CfSpan)
	{
		Winding3 f, b;
		w.split(p, f, b);
		if (!f.empty())
		{
			if (n.front >= 0)
				clip(n.front, f, true, visitor);
			else
				visitor(n.plane, f, Winding3::CfFront, true);
		}
		if (!b.empty())
		{
			if (n.back >= 0)
				clip(n.back, b, true, visitor);
			else
				visitor(n.plane, b, Winding3::CfBack, true);
		}
	}
}

}

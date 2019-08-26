#include "Core/Log/Log.h"
#include "Core/Math/BspTree.h"
#include "Core/Math/Const.h"

namespace traktor
{

BspTree::BspTree()
{
}

BspTree::BspTree(const AlignedVector< Winding3 >& polygons)
{
	build(polygons);
}

bool BspTree::build(const AlignedVector< Winding3 >& polygons)
{
	if (polygons.empty())
		return false;

	AlignedVector< Winding3 > mutablePolygons = polygons;

	// Initially reserve equal amount of nodes as windings, probably more but never less.
	m_nodes.reserve(polygons.size());

	// Recursively build nodes.
	int32_t root = internalBuild(mutablePolygons);
	return (bool)(root >= 0);
}

bool BspTree::inside(const Vector4& pt) const
{
	return inside(0, pt);
}

bool BspTree::inside(const Winding3& w) const
{
	return inside(0, w);
}

void BspTree::clip(const Winding3& w, const std::function< void(const Winding3& w, uint32_t cl, bool splitted) >& visitor) const
{
	Plane p;
	if (!w.getPlane(p))
		return;
	clip(0, w, p, false, visitor);
}

int32_t BspTree::internalBuild(AlignedVector< Winding3 >& polygons)
{
	int32_t node = (int32_t)m_nodes.size();
	Node& n = m_nodes.push_back();
	
	if (!polygons[0].getPlane(n.plane))
		return -1;

	AlignedVector< Winding3 > frontPolygons, backPolygons;
	for (size_t i = 1; i < polygons.size(); ++i)
	{
		int32_t cf = polygons[i].classify(n.plane);
		if (cf == Winding3::CfFront || cf == Winding3::CfCoplanar)
		{
			frontPolygons.push_back(polygons[i]);
		}
		else if (cf == Winding3::CfBack)
		{
			backPolygons.push_back(polygons[i]);
		}
		else
		{
			T_ASSERT(cf == Winding3::CfSpan);

			Winding3& f = frontPolygons.push_back();
			Winding3& b = backPolygons.push_back();
			polygons[i].split(n.plane, f, b);

			T_FATAL_ASSERT(f.size() >= 3);
			T_FATAL_ASSERT(b.size() >= 3);
		}
	}

	T_ASSERT(frontPolygons.size() == frontPlanes.size());
	T_ASSERT(backPolygons.size() == backPlanes.size());

	// Discard input windings here; not used any more and
	// we don't want an memory explosion.
	polygons.clear();

	if (!frontPolygons.empty())
		m_nodes[node].front = internalBuild(frontPolygons);
	if (!backPolygons.empty())
		m_nodes[node].back = internalBuild(backPolygons);

	return node;
}

bool BspTree::inside(int32_t node, const Vector4& pt) const
{
	const Node& n = m_nodes[node];
	float d = n.plane.distance(pt);
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

	int cf = w.classify(n.plane);
	if (cf == Winding3::CfFront || cf == Winding3::CfCoplanar)
		result = (n.front >= 0) ? inside(n.front, w) : true;
	else if (cf == Winding3::CfBack)
		result = (n.back >= 0) ? inside(n.back, w) : false;
	else
	{
		T_ASSERT(cf == Winding3::CfSpan);
		Winding3 f, b;

		w.split(n.plane, f, b);
		T_ASSERT(!f.empty());
		T_ASSERT(!b.empty());

		if (n.front >= 0)
			result |= inside(n.front, f);
		if (n.back >= 0)
			result |= inside(n.back, b);
	}

	return result;
}

void BspTree::clip(int32_t node, const Winding3& w, const Plane& wp, bool splitted, const std::function< void(const Winding3& w, uint32_t cl, bool splitted) >& visitor) const
{
	const Node& n = m_nodes[node];

	if (std::abs(w.area()) < FUZZY_EPSILON)
		return;

	int32_t cf = w.classify(n.plane);
	if (cf == Winding3::CfCoplanar)
		cf = dot3(n.plane.normal(), wp.normal()) >= 0.0f ? Winding3::CfFront : Winding3::CfBack;

	if (cf == Winding3::CfFront)
	{
		if (n.front >= 0)
			clip(n.front, w, wp, splitted, visitor);
		else
			visitor(w, Winding3::CfFront, splitted);
	}
	else if (cf == Winding3::CfBack)
	{
		if (n.back >= 0)
			clip(n.back, w, wp, splitted, visitor);
		else
			visitor(w, Winding3::CfBack, splitted);
	}
	else if (cf == Winding3::CfSpan)
	{
		Winding3 f, b;
		w.split(n.plane, f, b);
		T_FATAL_ASSERT(!f.empty());
		T_FATAL_ASSERT(!b.empty());
		if (!f.empty())
		{
			if (n.front >= 0)
				clip(n.front, f, wp, true, visitor);
			else
				visitor(f, Winding3::CfFront, true);
		}
		if (!b.empty())
		{
			if (n.back >= 0)
				clip(n.back, b, wp, true, visitor);
			else
				visitor(b, Winding3::CfBack, true);
		}
	}
}

}

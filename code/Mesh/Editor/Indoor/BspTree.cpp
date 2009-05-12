#include "Mesh/Editor/Indoor/BspTree.h"
#include "Core/Heap/HeapNew.h"

namespace traktor
{
	namespace mesh
	{

bool BspTree::build(const AlignedVector< Winding >& polygons)
{
	m_root = recursiveBuild(polygons);
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

BspTree::BspNode* BspTree::recursiveBuild(const AlignedVector< Winding >& polygons)
{
	BspNode* node = gc_new< BspNode >();
	
	node->plane = polygons[0].plane();

	AlignedVector< Winding > front, back;
	for (size_t i = 1; i < polygons.size(); ++i)
	{
		int cf = polygons[i].classify(node->plane);
		if (cf == Winding::CfCoplanar)
			cf = dot3(node->plane.normal(), polygons[i].plane().normal()) >= 0.0f ? Winding::CfFront : Winding::CfBack;

		if (cf == Winding::CfFront)
			front.push_back(polygons[i]);
		else if (cf == Winding::CfBack)
			back.push_back(polygons[i]);
		else if (cf == Winding::CfSpan)
		{
			Winding f, b;
			
			polygons[i].split(node->plane, f, b);
			T_ASSERT (!f.points.empty());
			T_ASSERT (!b.points.empty());

			if (f.points.size() >= 3)
				front.push_back(f);
			if (b.points.size() >= 3)
				back.push_back(b);
		}
	}

	if (!front.empty())
		node->front = recursiveBuild(front);
	if (!back.empty())
		node->back = recursiveBuild(back);

	return node;
}

bool BspTree::inside(const BspNode* node, const Vector4& pt) const
{
	float d = node->plane.distance(pt);
	if (d >= 0.0f)
		return node->front ? inside(node->front, pt) : true;
	else
		return node->back ? inside(node->back, pt) : false;
}

bool BspTree::inside(const BspNode* node, const Winding& w) const
{
	bool result = false;

	int cf = w.classify(node->plane);

	if (cf == Winding::CfCoplanar)
		cf = dot3(node->plane.normal(), w.plane().normal()) >= 0.0f ? Winding::CfFront : Winding::CfBack;

	if (cf == Winding::CfFront)
		result = node->front ? inside(node->front, w) : true;
	else if (cf == Winding::CfBack)
		result = node->back ? inside(node->back, w) : false;
	else if (cf == Winding::CfSpan)
	{
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
}

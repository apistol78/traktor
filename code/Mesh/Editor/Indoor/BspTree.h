#ifndef traktor_mesh_BspTree_H
#define traktor_mesh_BspTree_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Winding.h"

namespace traktor
{
	namespace mesh
	{

class BspTree
{
public:
	bool build(const AlignedVector< Winding >& polygons);

	bool inside(const Vector4& pt) const;

	bool inside(const Winding& w) const;

	template < typename PolygonType >
	void clip(const PolygonType& polygon, AlignedVector< PolygonType >& outClipped) const
	{
		T_ASSERT (m_root);
		clip< PolygonType >(m_root, polygon, outClipped);
	}

private:
	struct BspNode : public Object
	{
		Plane plane;
		Ref< BspNode > front;
		Ref< BspNode > back;
	};

	Ref< BspNode > m_root;

	BspNode* recursiveBuild(const AlignedVector< Winding >& polygons);

	bool inside(const BspNode* node, const Vector4& pt) const;

	bool inside(const BspNode* node, const Winding& w) const;

	template < typename PolygonType >
	void clip(const BspNode* node, const PolygonType& polygon, AlignedVector< PolygonType >& outClipped) const
	{
		Winding w = polygon.winding();

		int cf = w.classify(node->plane);
		if (cf == Winding::CfCoplanar)
			cf = dot3(node->plane.normal(), w.plane().normal()) >= 0.0f ? Winding::CfFront : Winding::CfBack;

		if (cf == Winding::CfFront)
		{
			if (node->front)
				clip(node->front, polygon, outClipped);
			else if (polygon.valid())
				outClipped.push_back(polygon);
		}
		else if (cf == Winding::CfBack)
		{
			if (node->back)
				clip(node->back, polygon, outClipped);
		}
		else if (cf == Winding::CfSpan)
		{
			PolygonType f, b;
			polygon.split(node->plane, f, b);
			if (f.valid())
			{
				if (node->front)
					clip(node->front, f, outClipped);
				else
					outClipped.push_back(f);
			}
			if (b.valid())
			{
				if (node->back)
					clip(node->back, b, outClipped);
			}
		}
	}
};

	}
}

#endif	// traktor_mesh_BspTree_H

#include <deque>
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const float c_normalThreshold = 0.01f;
const float c_distanceThreshold = 0.05f;

bool comparePlanesEqual(const Plane& lh, const Plane& rh)
{
	if (dot3(lh.normal(), rh.normal()) < Scalar(1.0f - c_normalThreshold))
		return false;

	if (abs(lh.distance() - rh.distance()) > Scalar(c_distanceThreshold))
		return false;

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeCoplanarAdjacents", MergeCoplanarAdjacents, IModelOperation)

MergeCoplanarAdjacents::MergeCoplanarAdjacents(bool allowConvexOnly)
:	m_allowConvexOnly(allowConvexOnly)
{
}

bool MergeCoplanarAdjacents::apply(Model& model) const
{
	AlignedVector< Plane > planes;
	SmallMap< uint32_t, std::vector< uint32_t > > planePolygons;
	Winding3 winding;
	Plane plane;
	Model result;

	// 	Group polygons which share same plane.
	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const Polygon& polygon = model.getPolygon(i);

		winding.points.resize(polygon.getVertexCount());
		for (uint32_t j = 0; j < polygon.getVertexCount(); ++j)
			winding.points[j] = model.getVertexPosition(polygon.getVertex(j));

		if (winding.getPlane(plane))
		{
			bool foundPlane = false;
			for (uint32_t j = 0; j < planes.size(); ++j)
			{
				if (comparePlanesEqual(planes[j], plane))
				{
					planePolygons[j].push_back(i);
					foundPlane = true;
					break;
				}
			}
			if (!foundPlane)
			{
				planes.push_back(plane);
				planePolygons[planes.size() - 1].push_back(i);
			}
		}
	}

	// For each plane, find isolated groups of polygons which are connected.
	for (SmallMap< uint32_t, std::vector< uint32_t > >::const_iterator i = planePolygons.begin(); i != planePolygons.end(); ++i)
	{
		const Plane& plane = planes[i->first];
		const std::vector< uint32_t >& polygonIds = i->second;
		std::deque< uint32_t > edgeHeap;

		// Create adjacency information only on polygons on plane.
		ModelAdjacency adjacency(&model, polygonIds, ModelAdjacency::MdByPosition);

		// Create a heap of un-shared edges.
		for (uint32_t j = 0; j < polygonIds.size(); ++j)
		{
			const Polygon& polygon = model.getPolygon(polygonIds[j]);
			const std::vector< uint32_t >& vertexIds = polygon.getVertices();
			for (uint32_t k = 0; k < vertexIds.size(); ++k)
			{
				uint32_t edge = adjacency.getEdge(polygonIds[j], k);
				if (adjacency.getSharedEdgeCount(edge) == 0)
					edgeHeap.push_back(edge);
			}
		}

		// Create edge loops.
		while (!edgeHeap.empty())
		{
			std::vector< uint32_t > outline;

			uint32_t edge = edgeHeap.front();
			edgeHeap.pop_front();

			uint32_t index1a, index1b;
			adjacency.getEdgeIndices(edge, index1a, index1b);
			outline.push_back(index1a);

			while (!edgeHeap.empty())
			{
				bool foundEdge = false;
				for (std::deque< uint32_t >::iterator i = edgeHeap.begin(); i != edgeHeap.end(); ++i)
				{
					uint32_t index2a, index2b;
					adjacency.getEdgeIndices(*i, index2a, index2b);

					if (index1b == index2a)
					{
						edge = *i;
						edgeHeap.erase(i);
						foundEdge = true;
						break;
					}
				}
				if (!foundEdge)
					break;

				adjacency.getEdgeIndices(edge, index1a, index1b);
				outline.push_back(index1a);

				if (index1b == outline.front())
					break;
			}

			if (outline.size() >= 3)
			{
				// \note 1) if outline has opposite winding then it's a hole.
				// \note 2) possibly able to remove colinear vertices from winding; "should" be safe.

				Winding3 outlineWinding;
				for (std::vector< uint32_t >::const_iterator i = outline.begin(); i != outline.end(); ++i)
				{
					const Vector4& position = model.getPosition(*i);
					outlineWinding.points.push_back(position);
				}

				Plane outlinePlane;
				if (outlineWinding.getPlane(outlinePlane))
				{
					if (dot3(outlinePlane.normal(), plane.normal()) > 0.0f)
					{
						for (uint32_t i = 0; i < outlineWinding.points.size(); )
						{
							uint32_t i1 = (i + 1) % outlineWinding.points.size();
							uint32_t i2 = (i + 2) % outlineWinding.points.size();

							const Vector4& x0 = outlineWinding.points[i1];
							const Vector4& x1 = outlineWinding.points[i];
							const Vector4& x2 = outlineWinding.points[i2];

							Vector4 A = x2 - x1;
							Scalar Aln = A.length();

							if (Aln < c_distanceThreshold)
							{
								// Too short edge.
								outlineWinding.points.erase(outlineWinding.points.begin() + i);
								outlineWinding.points.erase(outlineWinding.points.begin() + i);
								continue;
							}

							Vector4 B = x1 - x0;
							Scalar Bln = B.length();

							if (Bln < c_distanceThreshold)
							{
								// Point to close.
								outlineWinding.points.erase(outlineWinding.points.begin() + i1);
								continue;

							}

							Scalar d = cross(A, x1 - x0).length() / Aln;
							if (d < c_distanceThreshold)
							{
								// Point collinear.
								outlineWinding.points.erase(outlineWinding.points.begin() + i1);
								continue;
							}

							++i;
						}

						if (outlineWinding.points.size() >= 3)
						{
							Polygon outlinePolygon;
							for (AlignedVector< Vector4 >::const_iterator i = outlineWinding.points.begin(); i != outlineWinding.points.end(); ++i)
							{
								Vertex outlineVertex;
								outlineVertex.setPosition(result.addUniquePosition(*i));
								outlinePolygon.addVertex(result.addUniqueVertex(outlineVertex));
							}
							result.addPolygon(outlinePolygon);
						}
					}
				}
			}
		}
	}

	// Replace model with result.
	model = result;
	return true;
}

	}
}

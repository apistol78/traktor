#include <functional>
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

const float c_planarAngleThreshold = deg2rad(1.0f);
const float c_vertexDistanceThreshold = 0.1f;

struct NoVerticesPred
{
	bool operator () (const Polygon& pol) const
	{
		return pol.getVertexCount() == 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeCoplanarAdjacents", MergeCoplanarAdjacents, IModelOperation)

MergeCoplanarAdjacents::MergeCoplanarAdjacents(bool allowConvexOnly)
:	m_allowConvexOnly(allowConvexOnly)
{
}

bool MergeCoplanarAdjacents::apply(Model& model) const
{
	std::vector< Polygon >& polygons = model.getPolygons();
	Winding3 w;
	Plane p;

	// Calculate polygon normals.
	AlignedVector< Vector4 > normals(polygons.size(), Vector4(0.0f, 0.0f, 1.0f));
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];
		if (polygon.getVertexCount() < 3)
			continue;

		w.clear();
		for (uint32_t j = 0; j < polygon.getVertexCount(); ++j)
			w.push(model.getVertexPosition(polygon.getVertex(j)));

		if (w.getPlane(p))
			normals[i] = p.normal();
	}

	// Build model adjacency information.
	ModelAdjacency adjacency(&model, ModelAdjacency::MdByPosition);
	std::vector< uint32_t > sharedEdges;
	std::vector< uint32_t > removeIndices;

	// Keep iterating until no more polygons are merged.
	for (;;)
	{
		int32_t merged = 0;
		for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
		{
			Polygon& leftPolygon = polygons[i];
			for (uint32_t j = 0; j < leftPolygon.getVertexCount(); ++j)
			{
				adjacency.getSharedEdges(i, j, sharedEdges);
				if (sharedEdges.size() != 1)
					continue;

				uint32_t sharedEdge = sharedEdges.front();
				uint32_t sharedPolygon = adjacency.getPolygon(sharedEdge);
				
				// In case the source model contain internal edges or something
				// gets broken during merging.
				if (sharedPolygon == i)
					continue;

				float phi = acos(abs(dot3(normals[i], normals[sharedPolygon])));
				if (phi <= c_planarAngleThreshold)
				{
					Polygon& rightPolygon = polygons[sharedPolygon];

					std::vector< uint32_t > leftVertices = leftPolygon.getVertices();
					std::vector< uint32_t > rightVertices = rightPolygon.getVertices();

					if (leftVertices.size() < 3 || rightVertices.size() < 3)
						continue;

					// Rotate polygons so sharing edge is first on both left and right polygon.
					std::rotate(leftVertices.begin(), leftVertices.begin() + j, leftVertices.end());
					std::rotate(rightVertices.begin(), rightVertices.begin() + adjacency.getPolygonEdge(sharedEdge), rightVertices.end());

					// Merge polygons, except sharing edges.
					std::vector< uint32_t > mergedVertices;
					mergedVertices.insert(mergedVertices.end(), rightVertices.begin() + 1, rightVertices.end());
					mergedVertices.insert(mergedVertices.end(), leftVertices.begin() + 1, leftVertices.end());
					if (mergedVertices.size() <= 2)
						continue;

					// Remove internal loops.
					for (uint32_t k = 0; k < mergedVertices.size(); ++k)
					{
						uint32_t i0 = mergedVertices[k];
						uint32_t i1 = mergedVertices[(k + 2) % mergedVertices.size()];
						if (i0 == i1)
						{
							mergedVertices.erase(mergedVertices.begin() + k);
							mergedVertices.erase(mergedVertices.begin() + k % mergedVertices.size());
							break;
						}
					}

					// Remove non-silhouette vertices.
					removeIndices.resize(0);

					for (uint32_t i0 = 0; i0 < mergedVertices.size(); ++i0)
					{
						uint32_t i1 = (i0 + 1) % mergedVertices.size();
						uint32_t i2 = (i0 + 2) % mergedVertices.size();

						const Vector4& v0 = model.getVertexPosition(mergedVertices[i0]);
						const Vector4& v1 = model.getVertexPosition(mergedVertices[i1]);
						const Vector4& v2 = model.getVertexPosition(mergedVertices[i2]);

						Vector4 v0_2 = (v2 - v0).xyz0();
						Vector4 v0_1 = (v1 - v0).xyz0();

						Scalar k = dot3(v0_2, v0_1) / v0_2.length2();
						if (k >= 0.0f && k <= 1.0f)
						{
							Vector4 vp = v0 + v0_2 * k;
							Scalar dist = (v1 - vp).xyz0().length();
							if (dist <= c_vertexDistanceThreshold)
								removeIndices.push_back(i1);
						}
					}

					if (!removeIndices.empty())
					{
						std::sort(removeIndices.begin(), removeIndices.end(), std::greater< uint32_t >());
						for (std::vector< uint32_t >::const_iterator it = removeIndices.begin(); it != removeIndices.end(); ++it)
							mergedVertices.erase(mergedVertices.begin() + *it);
					}

					// Ensure merged polygon is still convex.
					if (m_allowConvexOnly)
					{
						uint32_t sign = 0;
						for (uint32_t i0 = 0; i0 < mergedVertices.size() && sign != 3; ++i0)
						{
							uint32_t i1 = (i0 + 1) % mergedVertices.size();
							uint32_t i2 = (i0 + 2) % mergedVertices.size();

							const Vector4& v0 = model.getVertexPosition(mergedVertices[i0]);
							const Vector4& v1 = model.getVertexPosition(mergedVertices[i1]);
							const Vector4& v2 = model.getVertexPosition(mergedVertices[i2]);

							Vector4 v0_1 = (v1 - v0).xyz0();
							Vector4 v1_2 = (v2 - v1).xyz0();

							float phi = dot3(cross(v0_1, v1_2), normals[i]);
							if (phi < -FUZZY_EPSILON)
								sign |= 1;
							else if (phi > FUZZY_EPSILON)
								sign |= 2;
						}
						if (sign == 3)
							continue;
					}
					
					// Set all vertices in left polygon and null out right polygon.
					leftPolygon.setVertices(mergedVertices);
					rightPolygon.setVertices(std::vector< uint32_t >());

					// Re-build adjacency.
					adjacency.remove(sharedPolygon);
					adjacency.remove(i);
					adjacency.add(i);

					++merged;
					break;
				}
			}
		}
		if (merged == 0)
			break;
	}

	// Remove all polygons with no vertices.
	std::vector< Polygon >::iterator it = std::remove_if(polygons.begin(), polygons.end(), NoVerticesPred());
	polygons.erase(it, polygons.end());

	// Cleanup model from unused vertices etc.
	if (!CleanDuplicates(0.1f).apply(model))
		return false;

	return true;
}

	}
}

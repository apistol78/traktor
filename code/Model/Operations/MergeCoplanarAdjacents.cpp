/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

const float c_planarAngleThreshold = deg2rad(0.1f);
const float c_vertexDistanceThreshold = 0.001f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeCoplanarAdjacents", MergeCoplanarAdjacents, IModelOperation)

bool MergeCoplanarAdjacents::apply(Model& model) const
{
	AlignedVector< Polygon >& polygons = model.getPolygons();
	Winding3 w;
	Plane p;

	// Calculate polygon normals.
	AlignedVector< Vector4 > normals(polygons.size(), Vector4(0.0f, 0.0f, 1.0f));
	for (size_t i = 0; i < polygons.size(); ++i)
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
	ModelAdjacency adjacency(&model, ModelAdjacency::Mode::ByPosition);
	ModelAdjacency::share_vector_t sharedEdges;
	AlignedVector< uint32_t > removeIndices;

	// Keep iterating until no more polygons are merged.
	for (;;)
	{
		int32_t merged = 0;
		for (size_t i = 0; i < polygons.size(); ++i)
		{
			Polygon& leftPolygon = polygons[i];
			for (size_t j = 0; j < leftPolygon.getVertexCount(); ++j)
			{
				adjacency.getSharedEdges(i, j, sharedEdges);
				if (sharedEdges.size() != 1)
					continue;

				uint32_t sharedEdge = sharedEdges.front();
				uint32_t sharedPolygon = adjacency.getPolygon(sharedEdge);

				if (sharedPolygon <= i)
					continue;

				float phi = acos(abs(dot3(normals[i], normals[sharedPolygon])));
				if (phi <= c_planarAngleThreshold)
				{
					Polygon& rightPolygon = polygons[sharedPolygon];

					auto leftVertices = leftPolygon.getVertices();
					auto rightVertices = rightPolygon.getVertices();

					if (leftVertices.size() < 3 || rightVertices.size() < 3)
						continue;

					// Rotate polygons so sharing edge is first on both left and right polygon.
					std::rotate(leftVertices.begin(), leftVertices.begin() + j, leftVertices.end());
					std::rotate(rightVertices.begin(), rightVertices.begin() + (size_t)adjacency.getPolygonEdge(sharedEdge), rightVertices.end());

					// Merge polygons, except sharing edges.
					AlignedVector< uint32_t > mergedVertices;
					mergedVertices.insert(mergedVertices.end(), rightVertices.begin() + 1, rightVertices.end());
					mergedVertices.insert(mergedVertices.end(), leftVertices.begin() + 1, leftVertices.end());
					if (mergedVertices.size() <= 2)
						continue;

					// Remove internal loops.
					for (size_t k = 0; k < mergedVertices.size(); ++k)
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
					for (size_t i0 = 0; i0 < mergedVertices.size(); ++i0)
					{
						size_t i1 = (i0 + 1) % mergedVertices.size();
						size_t i2 = (i0 + 2) % mergedVertices.size();

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
						for (const auto removeIndex : removeIndices)
							mergedVertices.erase(mergedVertices.begin() + (size_t)removeIndex);
					}

					// Set all vertices in left polygon and null out right polygon.
					leftPolygon.setVertices(Polygon::vertices_t(mergedVertices.begin(), mergedVertices.end()));
					rightPolygon.clearVertices();

					// Re-build adjacency.
					adjacency.remove(sharedPolygon);
					adjacency.update(i);

					++merged;
					break;
				}
			}
		}
		if (merged == 0)
			break;
	}

	// Remove all polygons with no vertices.
	AlignedVector< Polygon >::iterator it = std::remove_if(polygons.begin(), polygons.end(), [](const Polygon& pol) {
		return pol.getVertexCount() == 0;
	});
	polygons.erase(it, polygons.end());

	// Cleanup model from unused vertices etc.
	if (!CleanDuplicates(0.001f).apply(model))
		return false;

	return true;
}

	}
}

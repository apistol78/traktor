/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/Operations/FlattenDoubleSided.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.FlattenDoubleSided", FlattenDoubleSided, IModelOperation)

bool FlattenDoubleSided::apply(Model& model) const
{
	const AlignedVector< Polygon >& polygons = model.getPolygons();
	Winding3 w;
	Plane p;

	AlignedVector< Polygon > flatten;
	flatten.reserve(polygons.size());

	for (const auto& polygon : polygons)
	{
		const uint32_t materialId = polygon.getMaterial();
		const Material& material = model.getMaterial(materialId);
		if (!material.isDoubleSided())
			continue;

		Polygon flat = polygon;

		if (flat.getNormal() != c_InvalidIndex)
		{
			const Vector4 sourceNormal = model.getNormal(flat.getNormal());
			const Vector4 flattenNormal = -sourceNormal;
			flat.setNormal(model.addUniqueNormal(flattenNormal));
		}

		for (uint32_t i = 0; i < polygon.getVertexCount(); ++i)
		{
			const uint32_t sourceVertexId = polygon.getVertex(i);

			const Vertex& sourceVertex = model.getVertex(sourceVertexId);
			Vertex flattenVertex = sourceVertex;

			if (sourceVertex.getNormal() != c_InvalidIndex)
			{
				const Vector4 sourceNormal = model.getNormal(sourceVertex.getNormal());
				const Vector4 flattenNormal = -sourceNormal;
				flattenVertex.setNormal(model.addUniqueNormal(flattenNormal));
			}

			const uint32_t flattenVertexId = model.addUniqueVertex(flattenVertex);
			flat.setVertex(polygon.getVertexCount() - i - 1, flattenVertexId);
		}

		flatten.push_back(flat);
	}

	flatten.insert(flatten.end(), polygons.begin(), polygons.end());
	model.setPolygons(flatten);

	AlignedVector< Material > materials = model.getMaterials();
	for (auto& material : materials)
		material.setDoubleSided(false);
	model.setMaterials(materials);
	return true;
}

}

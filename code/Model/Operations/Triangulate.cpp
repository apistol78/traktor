/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/Operations/Triangulate.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Triangulate", Triangulate, IModelOperation)

bool Triangulate::required(const IModelOperation* lastOperation) const
{
	return &type_of(lastOperation) != &type_of(this);
}

bool Triangulate::apply(Model& model) const
{
	AlignedVector< Polygon > triangulatedPolygons;
	Winding3 polygonWinding;

	const auto& polygons = model.getPolygons();
	triangulatedPolygons.reserve(polygons.size());

	for (const auto& polygon : polygons)
	{
		const auto& vertices = polygon.getVertices();
		if (vertices.size() > 3)
		{
			polygonWinding.resize(0);
			for (size_t vertex : vertices)
				polygonWinding.push(model.getVertexPosition((uint32_t)vertex));

			Vector4 polygonNormal;
			if (polygon.getNormal() != c_InvalidIndex)
				polygonNormal = model.getNormal(polygon.getNormal());
			else
			{
				// No normal associated with polygon; try to determine from winding.
				Plane polygonPlane;
				if (!polygonWinding.getPlane(polygonPlane))
					continue;

				polygonNormal = polygonPlane.normal();
			}

			Triangulator().freeze(
				polygonWinding.get(),
				polygonNormal,
				Triangulator::Mode::Sorted,
				[&](size_t i0, size_t i1, size_t i2) {
					triangulatedPolygons.push_back(Polygon(
						polygon.getMaterial(),
						vertices[i0],
						vertices[i1],
						vertices[i2]
					));
				}
			);
		}
		else if (vertices.size() == 3)
			triangulatedPolygons.push_back(polygon);
	}

	model.setPolygons(triangulatedPolygons);
	return true;
}

}

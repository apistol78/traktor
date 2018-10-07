/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/Operations/Triangulate.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Triangulate", Triangulate, IModelOperation)

bool Triangulate::apply(Model& model) const
{
	AlignedVector< Polygon > triangulatedPolygons;
	AlignedVector< Triangulator::Triangle > triangles;

	const AlignedVector< Polygon >& polygons = model.getPolygons();
	triangulatedPolygons.reserve(polygons.size());

	for (AlignedVector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const AlignedVector< uint32_t >& vertices = i->getVertices();
		if (vertices.size() > 3)
		{
			Winding3 polygonWinding;

			for (size_t j = 0; j < vertices.size(); ++j)
				polygonWinding.push(model.getPosition(model.getVertex(vertices[j]).getPosition()));

			Vector4 polygonNormal;
			if (i->getNormal() != c_InvalidIndex)
				polygonNormal = model.getNormal(i->getNormal());
			else
			{
				// No normal associated with polygon; try to determine from winding.
				Plane polygonPlane;
				if (!polygonWinding.getPlane(polygonPlane))
					continue;

				polygonNormal = polygonPlane.normal();
			}

			triangles.resize(0);
			Triangulator().freeze(
				polygonWinding.getPoints(),
				polygonNormal,
				triangles
			);

			for (AlignedVector< Triangulator::Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
			{
				Polygon triangulatedPolygon(
					i->getMaterial(),
					vertices[j->indices[0]],
					vertices[j->indices[1]],
					vertices[j->indices[2]]
				);
				triangulatedPolygons.push_back(triangulatedPolygon);
			}
		}
		else if (vertices.size() == 3)
			triangulatedPolygons.push_back(*i);
	}

	model.setPolygons(triangulatedPolygons);
	return true;
}

	}
}

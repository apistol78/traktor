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

	const auto& polygons = model.getPolygons();
	triangulatedPolygons.reserve(polygons.size());

	for (const auto& polygon : polygons)
	{
		const auto& vertices = polygon.getVertices();
		if (vertices.size() > 3)
		{
			Winding3 polygonWinding;

			for (size_t vertex : vertices)
				polygonWinding.push(model.getVertexPosition(vertex));

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
				Triangulator::TfSorted,
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
}

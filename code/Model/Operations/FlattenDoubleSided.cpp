#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/Operations/FlattenDoubleSided.h"

namespace traktor
{
	namespace model
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
		uint32_t materialId = polygon.getMaterial();
		const Material& material = model.getMaterial(materialId);
		if (!material.isDoubleSided())
			continue;

		Polygon flat = polygon;

		w.clear();
		for (uint32_t j = 0; j < polygon.getVertexCount(); ++j)
			w.push(model.getVertexPosition(polygon.getVertex(j)));
		if (!w.getPlane(p))
			continue;

		for (uint32_t i = 0; i < polygon.getVertexCount(); ++i)
		{
			uint32_t sourceVertexId = polygon.getVertex(i);

			const Vertex& sourceVertex = model.getVertex(sourceVertexId);
			Vertex flattenVertex = sourceVertex;

			if (sourceVertex.getNormal() != c_InvalidIndex)
			{
				Vector4 sourceNormal = model.getNormal(sourceVertex.getNormal());
				Vector4 flattenNormal = sourceNormal - p.normal() * (2.0_simd * dot3(p.normal(), sourceNormal));
				flattenVertex.setNormal(model.addUniqueNormal(flattenNormal));
			}

			uint32_t flattenVertexId = model.addUniqueVertex(flattenVertex);
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
}

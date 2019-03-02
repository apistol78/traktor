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
	AlignedVector< Polygon > flatten; flatten.reserve(polygons.size());

	for (AlignedVector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		uint32_t materialId = i->getMaterial();
		const Material& material = model.getMaterial(materialId);
		if (!material.isDoubleSided())
			continue;

		Polygon flat = *i;
		for (uint32_t j = 0; j < i->getVertexCount(); ++j)
		{
			uint32_t vtx = i->getVertex(j);
			flat.setVertex(i->getVertexCount() - j - 1, vtx);
		}
		flatten.push_back(flat);
	}

	flatten.insert(flatten.end(), polygons.begin(), polygons.end());
	model.setPolygons(flatten);

	AlignedVector< Material > materials = model.getMaterials();
	for (AlignedVector< Material >::iterator i = materials.begin(); i != materials.end(); ++i)
		i->setDoubleSided(false);

	model.setMaterials(materials);
	return true;
}

	}
}

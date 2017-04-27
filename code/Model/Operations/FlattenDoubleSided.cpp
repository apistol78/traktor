/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Model/Model.h"
#include "Model/Operations/FlattenDoubleSided.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.FlattenDoubleSided", FlattenDoubleSided, IModelOperation)

bool FlattenDoubleSided::apply(Model& model) const
{
	const std::vector< Polygon >& polygons = model.getPolygons();
	std::vector< Polygon > flatten; flatten.reserve(polygons.size());

	for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
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

	std::vector< Material > materials = model.getMaterials();
	for (std::vector< Material >::iterator i = materials.begin(); i != materials.end(); ++i)
		i->setDoubleSided(false);

	model.setMaterials(materials);
	return true;
}

	}
}

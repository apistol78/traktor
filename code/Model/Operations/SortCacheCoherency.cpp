/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Model/Model.h"
#include "Model/TriangleOrderForsyth.h"
#include "Model/Operations/SortCacheCoherency.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.SortCacheCoherency", SortCacheCoherency, IModelOperation)

bool SortCacheCoherency::apply(Model& model) const
{
	std::vector< Polygon > polygons = model.getPolygons();
	if (polygons.size() <= 2)
		return true;

	std::vector< Polygon > newPolygons;
	newPolygons.reserve(polygons.size());

	uint32_t materialCount = model.getMaterials().size();
	for (uint32_t material = 0; material < materialCount; ++material)
	{
		std::vector< uint32_t > indices;
		for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
		{
			if (i->getMaterial() == material)
				indices.insert(indices.end(), i->getVertices().begin(), i->getVertices().end());
		}

		if (indices.empty())
			continue;

		std::vector< uint32_t >::const_iterator it = std::max_element(indices.begin(), indices.end());
		uint32_t vertexCount = *it + 1;

		std::vector< uint32_t > newIndices(indices.size());
		optimizeFaces(
			indices,
			vertexCount,
			newIndices,
			32
		);

		for (uint32_t i = 0; i < newIndices.size(); i += 3)
		{
			newPolygons.push_back(Polygon(
				material,
				newIndices[i + 0],
				newIndices[i + 1],
				newIndices[i + 2]
			));
		}
	}

	model.setPolygons(newPolygons);
	return true;
}

	}
}

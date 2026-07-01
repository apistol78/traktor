/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Model/TriangleOrderForsyth.h"
#include "Model/Operations/SortCacheCoherency.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.SortCacheCoherency", SortCacheCoherency, IModelOperation)

bool SortCacheCoherency::apply(Model& model) const
{
	AlignedVector< Polygon > polygons = model.getPolygons();
	if (polygons.size() <= 2)
		return true;

	AlignedVector< Polygon > newPolygons;
	newPolygons.reserve(polygons.size());

	const uint32_t materialCount = (uint32_t)model.getMaterials().size();
	for (uint32_t material = 0; material < materialCount; ++material)
	{
		AlignedVector< uint32_t > indices;
		for (AlignedVector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
		{
			if (i->getMaterial() == material)
				indices.insert(indices.end(), i->getVertices().begin(), i->getVertices().end());
		}

		if (indices.empty())
			continue;

		AlignedVector< uint32_t >::const_iterator it = std::max_element(indices.begin(), indices.end());
		uint32_t vertexCount = *it + 1;

		AlignedVector< uint32_t > newIndices(indices.size());
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

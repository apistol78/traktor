/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDegenerate.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CleanDegenerate", CleanDegenerate, IModelOperation)

bool CleanDegenerate::apply(Model& model) const
{
	auto& polygons = model.getPolygons();
	for (size_t i = 0; i < polygons.size(); )
	{
		Polygon& polygon = polygons[i];

		auto& vertices = polygon.getVertices();
		if (vertices.size() > 1)
		{
			for (size_t j = 0; j < vertices.size(); )
			{
				const uint32_t p0 = model.getVertex(vertices[j]).getPosition();
				const uint32_t p1 = model.getVertex(vertices[(j + 1) % vertices.size()]).getPosition();
				if (p0 == p1)
					vertices.erase(vertices.begin() + j);
				else
					++j;
			}
		}

		if (vertices.size() <= 2)
			polygons.erase(polygons.begin() + i);
		else
			++i;
	}
	return true;
}

}

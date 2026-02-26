/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Model/Model.h"
#include "Model/Operations/MergeTVertices.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeTVertices", MergeTVertices, IModelOperation)

bool MergeTVertices::apply(Model& model) const
{
	const auto& modelVertices = model.getVertices();
	const auto& modelPositions = model.getPositions();

	for (auto& polygon : model.getPolygons())
	{
		auto polygonVertices = polygon.getVertices();

		uint32_t vc = (uint32_t)polygonVertices.size();
		for (uint32_t i = 0, j = vc - 1; i < vc; )
		{
			uint32_t v1 = polygonVertices[j];
			uint32_t v2 = polygonVertices[i];

			const Vertex& vx1 = model.getVertex(v1);
			const Vertex& vx2 = model.getVertex(v2);

			Vector4 p1 = modelPositions[vx1.getPosition()];
			Vector4 p2 = modelPositions[vx2.getPosition()];
			Vector4 d1 = p2 - p1;

			// Check if any vertex overlap edge segment.
			uint32_t ii = 0;
			for (; ii < modelVertices.size(); ++ii)
			{
				// Ensure vertex is not part of this polygon already.
				if (std::find(polygonVertices.begin(), polygonVertices.end(), ii) != polygonVertices.end())
					continue;

				Vector4 pc = modelPositions[modelVertices[ii].getPosition()];
				Vector4 d2 = pc - p1;

				// Check if close enough to edge.
				Vector4 c = cross(d1, d2);
				Scalar mn = c.length() / d1.length();
				if (mn > 0.0001f)
					continue;

				// Check if within edge vertices.
				Scalar d = dot3(d1, d2) / d1.length2();
				if (d <= 0.0f || d >= 1.0f)
					continue;

				// Found vertex.
				break;
			}
			if (ii >= modelVertices.size())
			{
				j = i++;
				continue;
			}

			// Found vertex overlapping polygon edge, add vertex to polygon.
			polygon.insertVertex(i, ii);

			j = i++;
			vc++;
		}
	}

	return true;
}

}

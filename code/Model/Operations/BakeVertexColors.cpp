/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Vector2i.h"
#include "Drawing/Image.h"
#include "Model/Model.h"
#include "Model/Operations/BakeVertexColors.h"

namespace traktor::model
{
	namespace
	{

float wrap(float n)
{
	return n - std::floor(n);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.BakeVertexColors", BakeVertexColors, IModelOperation)

bool BakeVertexColors::apply(Model& model) const
{
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		Vertex vtx = model.getVertex(i);

		if (vtx.getColor() != c_InvalidIndex || vtx.getTexCoord(0) == c_InvalidIndex)
			continue;

		Color4f acc(0.0f, 0.0f, 0.0f, 0.0f);
		int32_t count = 0;

		for (uint32_t j = 0; j < model.getPolygonCount(); ++j)
		{
			const Polygon& polygon = model.getPolygon(j);
			if (polygon.getMaterial() == c_InvalidIndex)
				continue;

			const auto& vertices = polygon.getVertices();
			if (std::find(vertices.begin(), vertices.end(), i) != vertices.end())
			{
				const Material& material = model.getMaterial(polygon.getMaterial());
				if (material.getDiffuseMap().image != nullptr)
				{
					const Vector2 size(
						material.getDiffuseMap().image->getWidth(),
						material.getDiffuseMap().image->getHeight()
					);

					const Vector2& uv = model.getTexCoord(vtx.getTexCoord(0));
					
					const Vector2i xy(
						(int32_t)(wrap(uv.x) * size.x),
						(int32_t)(wrap(uv.y) * size.y)
					);

					Color4f clr;
					if (material.getDiffuseMap().image->getPixel(xy.x, xy.y, clr))
					{
						acc += clr;
						count++;
					}
				}
				else
				{
					acc += material.getColor();
					count++;
				}
			}
		}

		if (count > 0)
		{
			acc /= Scalar((float)count);
			vtx.setColor(model.addUniqueColor(acc.rgb1()));
			model.setVertex(i, vtx);
		}
	}

	return true;
}

}

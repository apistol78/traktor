/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Model/Model.h"
#include "Shape/Editor/Bake/Embree/SplitModel.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

void splitPolygons(
	const model::Model* model,
	int32_t axis,
	float split,
	const AlignedVector< model::Polygon >& polygons,
	AlignedVector< model::Polygon >& outBack,
	AlignedVector< model::Polygon >& outFront
)
{
	outBack.reserve((2 * polygons.size()) / 3);
	outFront.reserve((2 * polygons.size()) / 3);

	for (const auto& polygon : polygons)
	{
		float range[2] = { std::numeric_limits< float >::max(), -std::numeric_limits< float >::max() };
		for (const auto& vertex : polygon.getVertices())
		{
			const Vector4& position = model->getVertexPosition(vertex);
			range[0] = std::min< float >(range[0], position[axis]);
			range[1] = std::max< float >(range[1], position[axis]);
		}

		if (range[0] >= split)
			outFront.push_back(polygon);
		else if (range[1] <= split)
			outBack.push_back(polygon);
		else
		{
			if (std::abs(range[0] - split) > std::abs(range[1] - split))
				outFront.push_back(polygon);
			else
				outBack.push_back(polygon);
		}
	}
}

void split(const model::Model* model, int32_t depth, const AlignedVector< model::Polygon >& polygons, RefArray< model::Model >& outModels)
{
	Aabb3 boundingBox;
	for (const auto& polygon : polygons)
	{
		for (const auto& vertex : polygon.getVertices())
			boundingBox.contain(model->getVertexPosition(vertex));
	}
	if (boundingBox.empty())
		return;

	const Vector4 center = boundingBox.getCenter();
	const Vector4 size = boundingBox.mx - boundingBox.mn;

	const static Scalar c_smallLimit = 2.0_simd;
	const bool smallBounds = (size.x() <= c_smallLimit && size.y() <= c_smallLimit && size.z() <= c_smallLimit);
	if (depth >= 3 || smallBounds)
	{
		Ref< model::Model > part = new model::Model();
		part->setPolygons(polygons);
		outModels.push_back(part);
		return;
	}

	const int32_t m = majorAxis3(size);

	AlignedVector< model::Polygon > backPolygons, frontPolygons;
	splitPolygons(model, m, center[m], polygons, backPolygons, frontPolygons);

	if (!backPolygons.empty())
	{
		split(
			model,
			depth + 1,
			backPolygons,
			outModels
		);
	}
	if (!frontPolygons.empty())
	{
		split(
			model,
			depth + 1,
			frontPolygons,
			outModels
		);
	}
}

		}

void splitModel(const model::Model* model, RefArray< model::Model >& outModels)
{
	split(model, 0, model->getPolygons(), outModels);
}

	}
}

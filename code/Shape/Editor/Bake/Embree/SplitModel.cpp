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

	const bool smallBounds = (size.x() <= 1.0_simd && size.y() <= 1.0_simd && size.z() <= 1.0_simd);
	if (depth >= 2 || smallBounds)
	{
		Ref< model::Model > part = new model::Model();
		part->setPolygons(polygons);
		outModels.push_back(part);
		return;
	}

	AlignedVector< model::Polygon > backPolygons[3], frontPolygons[3];
	AlignedVector< model::Polygon > octPolygons[8];

	splitPolygons(model, 0, center.x(), polygons, backPolygons[0], frontPolygons[0]);

	splitPolygons(model, 1, center.y(), backPolygons[0], backPolygons[1], frontPolygons[1]);
	splitPolygons(model, 1, center.y(), frontPolygons[0], backPolygons[2], frontPolygons[2]);

	splitPolygons(model, 2, center.z(), backPolygons[1], octPolygons[0], octPolygons[4]);
	splitPolygons(model, 2, center.z(), frontPolygons[1], octPolygons[2], octPolygons[6]);
	splitPolygons(model, 2, center.z(), backPolygons[2], octPolygons[1], octPolygons[5]);
	splitPolygons(model, 2, center.z(), frontPolygons[2], octPolygons[3], octPolygons[7]);

	for (int32_t i = 0; i < 8; ++i)
	{
		if (octPolygons[i].empty())
			continue;

		split(
			model,
			depth + 1,
			octPolygons[i],
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

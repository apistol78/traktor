/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "Core/Math/Triangle.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Model/Model.h"
#include "Model/ModelRasterizer.h"

namespace traktor::model
{
	namespace
	{

int32_t wrap(int32_t v, int32_t l)
{
	const int32_t c = v % l;
	return (c < 0) ? c + l : c;
}

	}


T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelRasterizer", ModelRasterizer, Object)

bool ModelRasterizer::generate(const Model* model, const Matrix44& modelView, drawing::Image* outImage) const
{
	const float hw = (float)(outImage->getWidth() / 2.0f);
	const float hh = (float)(outImage->getHeight() / 2.0f);
	const float r = (float)outImage->getWidth() / outImage->getHeight();

	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;

	const Matrix44 projection = perspectiveLh(deg2rad(70.0f), r, 0.1f, 100.0f);

	const auto& materials = model->getMaterials();
	const auto& polygons = model->getPolygons();
	const auto& vertices = model->getVertices();
	const auto& positions = model->getPositions();
	const auto& normals = model->getNormals();
	const auto& texCoords = model->getTexCoords();

	AlignedVector< float > zbuffer(outImage->getWidth() * outImage->getHeight(), 1.0f);

	for (const auto& polygon : polygons)
	{
		if (polygon.getMaterial() == model::c_InvalidIndex)
			continue;

		const auto& polygonVertices = polygon.getVertices();
		if (polygonVertices.size() != 3)
			continue;

		Vector4 cp[3];
		Vector4 nm[3];
		Vector2 sp[3];
		Vector2 uv[3];

		for (size_t i = 0; i < 3; ++i)
		{
			if (polygonVertices[i] >= vertices.size())
				return false;

			const model::Vertex& vertex = vertices[polygonVertices[i]];
			if (vertex.getPosition() == model::c_InvalidIndex || vertex.getNormal() == model::c_InvalidIndex)
				return false;

			const Vector4& position = positions[vertex.getPosition()];
			const Vector4& normal = normals[vertex.getNormal()];

			Vector4 vp = modelView * position;
			cp[i] = projection * vp;
			cp[i] = cp[i] / cp[i].w();

			nm[i] = (modelView * normal).normalized();

			sp[i] = Vector2(
				cp[i].x() * hw + hw,
				hh - (cp[i].y() * hh)
			);

			if (vertex.getTexCoord(0) != model::c_InvalidIndex)
				uv[i] = texCoords[vertex.getTexCoord(0)];
			else
				uv[i].set(0.0f, 0.0f);			
		}

		// Discard too large triangles, to prevent broken meshes to halt editor.
		const float bw = std::max< float >({ sp[0].x, sp[1].x, sp[2].x }) - std::min< float >({ sp[0].x, sp[1].x, sp[2].x });
		const float bh = std::max< float >({ sp[0].y, sp[1].y, sp[2].y }) - std::min< float >({ sp[0].y, sp[1].y, sp[2].y });
		if (bw > 2.0f * hw || bh > 2.0f * hh)
			continue;

		const auto& polygonMaterial = materials[polygon.getMaterial()];
		const drawing::Image* texture = polygonMaterial.getDiffuseMap().image;
		if (texture != nullptr)
		{
			const int32_t tw = texture->getWidth();
			const int32_t th = texture->getHeight();

			triangle(sp[0], sp[1], sp[2], [&, tw, th](int32_t x, int32_t y, float alpha, float beta, float gamma) {
				if (x < 0 || x >= outImage->getWidth() || y < 0 || y >= outImage->getHeight())
					return;

				const int32_t offset = x + y * outImage->getWidth();

				const float z = cp[0].z() * Scalar(alpha) + cp[1].z() * Scalar(beta) + cp[2].z() * Scalar(gamma);
				if (z < zbuffer[offset])
				{
					Color4f color;
					const Vector2 tc = uv[0] * alpha + uv[1] * beta + uv[2] * gamma;
					const int32_t tu = wrap((int32_t)(tc.x * tw), tw);
					const int32_t tv = wrap((int32_t)(tc.y * th), th);
					texture->getPixel(tu, tv, color);

					const Vector4 n = (nm[0] * Scalar(alpha) + nm[1] * Scalar(beta) + nm[2] * Scalar(gamma)).normalized();
					const Scalar d = -n.z() * 0.5_simd + 0.5_simd;

					outImage->setPixelUnsafe(x, y, (color * d * 2.0_simd).rgb1());
					zbuffer[offset] = z;
				}
			});
		}
		else
		{
			triangle(sp[0], sp[1], sp[2], [&](int32_t x, int32_t y, float alpha, float beta, float gamma) {
				if (x < 0 || x >= outImage->getWidth() || y < 0 || y >= outImage->getHeight())
					return;

				const int32_t offset = x + y * outImage->getWidth();

				const float z = cp[0].z() * Scalar(alpha) + cp[1].z() * Scalar(beta) + cp[2].z() * Scalar(gamma);
				if (z < zbuffer[offset])
				{
					const Color4f& color = polygonMaterial.getColor();
					const Vector4 n = (nm[0] * Scalar(alpha) + nm[1] * Scalar(beta) + nm[2] * Scalar(gamma)).normalized();
					const Scalar d = -n.z() * 0.5_simd + 0.5_simd;

					outImage->setPixelUnsafe(x, y, (color * d * 2.0_simd).rgb1());
					zbuffer[offset] = z;
				}
			});
		}
	}

	// Convert image from linear gamma to sRGB.
	drawing::GammaFilter gammaFilter(1.0f, 2.2f);
	outImage->apply(&gammaFilter);

	return true;
}

}

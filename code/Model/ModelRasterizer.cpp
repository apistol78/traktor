/*
 * TRAKTOR
 * Copyright (c) 2023-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/ModelRasterizer.h"

#include "Core/Math/Triangle.h"
#include "Drawing/Image.h"
#include "Model/Model.h"

#include <functional>

namespace traktor::model
{
namespace
{

const Vector4 c_sunDirection = Vector4(0.0f, -0.5f, 1.0f).normalized();

int32_t wrap(int32_t v, int32_t l)
{
	const int32_t c = v % l;
	return (c < 0) ? c + l : c;
}

Color4f lighting(const Vector4& p, const Vector4& n, const Color4f& materialColor, const Scalar& metalness, const Scalar& specularTerm)
{
	const Vector4 viewDirection = p.xyz0().normalized();
	const Vector4 halfWay = (c_sunDirection + viewDirection).normalized();
	const Scalar diffuse = clamp(dot3(c_sunDirection, -n), 0.4_simd, 1.0_simd) + 0.1_simd;
	const Scalar specular = power(abs(dot3(halfWay, -n)), 8.0_simd);
	return materialColor * diffuse * (1.0_simd - metalness) + lerp(Color4f(1.0f, 1.0f, 1.0f, 0.0f) * specularTerm, materialColor, metalness) * specular;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelRasterizer", ModelRasterizer, Object)

bool ModelRasterizer::generate(const Model* model, const Matrix44& modelView, drawing::Image* outImage) const
{
	const float r = (float)outImage->getWidth() / outImage->getHeight();
	return generate(model, modelView, perspectiveLh(deg2rad(70.0f), r, 0.1f, 100.0f), Options(), outImage);
}

bool ModelRasterizer::generate(const Model* model, const Matrix44& modelView, const Matrix44& projection, const Options& options, drawing::Image* outImage, drawing::Image* outNormalImage) const
{
	const int32_t width = outImage->getWidth();
	const int32_t height = outImage->getHeight();

	if (outNormalImage != nullptr && (outNormalImage->getWidth() != width || outNormalImage->getHeight() != height))
		return false;
	const float hw = (float)(width / 2.0f);
	const float hh = (float)(height / 2.0f);
	const Scalar alphaThreshold(options.alphaThreshold);

	const auto& materials = model->getMaterials();
	const auto& polygons = model->getPolygons();
	const auto& vertices = model->getVertices();
	const auto& positions = model->getPositions();
	const auto& normals = model->getNormals();
	const auto& texCoords = model->getTexCoords();

	AlignedVector< float > zbuffer(width * height, 1.0f);
	AlignedVector< Vector4 > positionsClip(positions.size());
	AlignedVector< Vector4 > normalsView(normals.size());

	(projection * modelView).transform(positions.c_ptr(), positionsClip.ptr(), positions.size());
	modelView.transform(normals.c_ptr(), normalsView.ptr(), normals.size());

	Vector4 cp[3];
	Vector4 nm[3];
	Vector2 sp[3];
	Vector2 uv[3];

	for (const auto& polygon : polygons)
	{
		if (polygon.getMaterial() == model::c_InvalidIndex)
			continue;

		const auto& polygonVertices = polygon.getVertices();
		if (polygonVertices.size() != 3)
			continue;

		for (size_t i = 0; i < 3; ++i)
		{
			if (polygonVertices[i] >= vertices.size())
				return false;

			const model::Vertex& vertex = vertices[polygonVertices[i]];
			if (vertex.getPosition() == model::c_InvalidIndex || vertex.getNormal() == model::c_InvalidIndex)
				return false;

			const Vector4& position = positionsClip[vertex.getPosition()];
			const Vector4& normal = normalsView[vertex.getNormal()];

			cp[i] = position / position.w();
			nm[i] = normal.normalized();
			sp[i] = Vector2(
				cp[i].x() * hw + hw,
				hh - (cp[i].y() * hh));

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
		const Scalar metalness = Scalar(polygonMaterial.getMetalness());
		const Scalar specularTerm = Scalar(polygonMaterial.getSpecularTerm());

		// Opaque materials commonly carry unrelated data in the alpha channel of their
		// diffuse map, so alpha is only meaningful for materials which ask for it.
		const bool materialAlpha =
			polygonMaterial.getBlendOperator() == Material::BoAlpha ||
			polygonMaterial.getBlendOperator() == Material::BoAlphaTest;

		const drawing::Image* texture = polygonMaterial.getDiffuseMap().image;
		const int32_t tw = (texture != nullptr) ? texture->getWidth() : 0;
		const int32_t th = (texture != nullptr) ? texture->getHeight() : 0;
		const bool textureLinear = (texture != nullptr && texture->getImageInfo() != nullptr) ? (std::abs(texture->getImageInfo()->getGamma() - 1.0f) < 0.1f) : false;

		// Rasterize the triangle with the given winding; the half-edge fill rule in
		// triangle() only emit fragments for one winding, so the reversed order pick up
		// exactly those polygons facing away from the camera. 'flip' turn the interpolated
		// normal back towards the camera for that pass.
		const auto rasterize = [&](int32_t i0, int32_t i1, int32_t i2, const Scalar& flip) {
			triangle(sp[i0], sp[i1], sp[i2], [&](int32_t x, int32_t y, float alpha, float beta, float gamma) {
				if (x < 0 || x >= width || y < 0 || y >= height)
					return;

				const int32_t offset = x + y * width;
				const Scalar salpha(alpha);
				const Scalar sbeta(beta);
				const Scalar sgamma(gamma);

				const Vector4 p = cp[i0] * salpha + cp[i1] * sbeta + cp[i2] * sgamma;
				if (p.z() >= zbuffer[offset])
					return;

				Color4f source;
				if (texture != nullptr)
				{
					const Vector2 tc = uv[i0] * alpha + uv[i1] * beta + uv[i2] * gamma;
					const int32_t tu = wrap((int32_t)(tc.x * tw), tw);
					const int32_t tv = wrap((int32_t)(tc.y * th), th);
					texture->getPixel(tu, tv, source);
				}
				else
					source = polygonMaterial.getColor();

				const Scalar sourceAlpha = materialAlpha ? source.getAlpha() : 1.0_simd;
				if (sourceAlpha < alphaThreshold)
					return;

				const Vector4 n = ((nm[i0] * salpha + nm[i1] * sbeta + nm[i2] * sgamma) * flip).normalized();

				Color4f d;
				if (options.unlit)
					d = source;
				else
				{
					const Color4f color = textureLinear ? source : source.linear();
					d = lighting(p, n, color, metalness, specularTerm).sRGB();
				}
				d.setAlpha(options.writeAlpha ? sourceAlpha : 1.0_simd);

				outImage->setPixelUnsafe(x, y, d);

				if (outNormalImage != nullptr)
				{
					// Encoded so the whole unit sphere fit an unsigned texture.
					const Vector4 encoded = n * 0.5_simd + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
					Color4f nd(encoded);
					nd.setAlpha(options.writeAlpha ? sourceAlpha : 1.0_simd);
					outNormalImage->setPixelUnsafe(x, y, nd);
				}

				zbuffer[offset] = p.z();
			});
		};

		rasterize(0, 1, 2, 1.0_simd);
		if (options.twoSided)
			rasterize(0, 2, 1, -1.0_simd);
	}

	return true;
}

}

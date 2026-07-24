/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/RayTracingMeshGeometry.h"

#include <algorithm>
#include <cmath>

#include "Core/Math/Color4f.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Material.h"
#include "Model/Model.h"

namespace traktor::mesh
{
namespace
{

const float c_alphaThreshold = 0.5f;		/*!< Alpha coverage threshold; texels with alpha below this are considered cut away. */
const float c_targetTexelsPerCell = 2.0f;	/*!< Target size, in texels, of a single tessellation cell along the longest UV edge. */
const int32_t c_maxSubdivisions = 32;		/*!< Upper bound on triangle subdivision to keep the generated geometry bounded. */

/*! Interpolated triangle corner used while clipping. */
struct RTCorner
{
	Vector4 position;
	Vector4 normal;
	Vector4 albedo;
	Vector2 texCoord;
	float weights[3];	//!< Barycentric weights within the source triangle.
	float alpha;
};

/*! Bilinearly sample the alpha channel of an image with wrap addressing. */
float sampleAlpha(const drawing::Image* image, const Vector2& uv)
{
	const int32_t w = image->getWidth();
	const int32_t h = image->getHeight();

	const float fx = uv.x * w - 0.5f;
	const float fy = uv.y * h - 0.5f;

	const int32_t x0 = (int32_t)std::floor(fx);
	const int32_t y0 = (int32_t)std::floor(fy);
	const float tx = fx - x0;
	const float ty = fy - y0;

	const auto wrap = [](int32_t v, int32_t n) -> int32_t {
		v %= n;
		if (v < 0)
			v += n;
		return v;
	};

	const int32_t wx0 = wrap(x0, w);
	const int32_t wy0 = wrap(y0, h);
	const int32_t wx1 = wrap(x0 + 1, w);
	const int32_t wy1 = wrap(y0 + 1, h);

	Color4f c00, c10, c01, c11;
	image->getPixelUnsafe(wx0, wy0, c00);
	image->getPixelUnsafe(wx1, wy0, c10);
	image->getPixelUnsafe(wx0, wy1, c01);
	image->getPixelUnsafe(wx1, wy1, c11);

	const float a0 = (float)c00.getAlpha() * (1.0f - tx) + (float)c10.getAlpha() * tx;
	const float a1 = (float)c01.getAlpha() * (1.0f - tx) + (float)c11.getAlpha() * tx;
	return a0 * (1.0f - ty) + a1 * ty;
}

/*! Construct a corner from barycentric weights within a source triangle, sampling coverage. */
RTCorner barycentricCorner(const RTCorner src[3], float w0, float w1, float w2, const drawing::Image* image)
{
	RTCorner c;
	c.position = src[0].position * Scalar(w0) + src[1].position * Scalar(w1) + src[2].position * Scalar(w2);
	c.normal = src[0].normal * Scalar(w0) + src[1].normal * Scalar(w1) + src[2].normal * Scalar(w2);
	c.albedo = src[0].albedo * Scalar(w0) + src[1].albedo * Scalar(w1) + src[2].albedo * Scalar(w2);
	c.texCoord = src[0].texCoord * w0 + src[1].texCoord * w1 + src[2].texCoord * w2;
	c.weights[0] = w0;
	c.weights[1] = w1;
	c.weights[2] = w2;
	c.alpha = sampleAlpha(image, c.texCoord);
	return c;
}

/*! Interpolate a corner where the coverage field crosses the threshold. */
RTCorner intersectCorner(const RTCorner& a, const RTCorner& b, float threshold)
{
	const float d = b.alpha - a.alpha;
	const float t = (std::abs(d) > 1e-8f) ? (threshold - a.alpha) / d : 0.0f;
	RTCorner c;
	c.position = lerp(a.position, b.position, Scalar(t));
	c.normal = lerp(a.normal, b.normal, Scalar(t));
	c.albedo = lerp(a.albedo, b.albedo, Scalar(t));
	c.texCoord = lerp(a.texCoord, b.texCoord, t);
	c.weights[0] = a.weights[0] + (b.weights[0] - a.weights[0]) * t;
	c.weights[1] = a.weights[1] + (b.weights[1] - a.weights[1]) * t;
	c.weights[2] = a.weights[2] + (b.weights[2] - a.weights[2]) * t;
	c.alpha = threshold;
	return c;
}

/*! Clip a triangle against the covered half-space (alpha >= threshold), treating
 *  the coverage as a linear field across the triangle. Returns the covered convex
 *  polygon (0, 3 or 4 corners). */
int32_t clipTriangleToCoverage(const RTCorner in[3], float threshold, RTCorner out[4])
{
	int32_t n = 0;
	for (int32_t i = 0; i < 3; ++i)
	{
		const RTCorner& cur = in[i];
		const RTCorner& nxt = in[(i + 1) % 3];
		const bool curIn = (cur.alpha >= threshold);
		const bool nxtIn = (nxt.alpha >= threshold);
		if (curIn)
			out[n++] = cur;
		if (curIn != nxtIn)
			out[n++] = intersectCorner(cur, nxt, threshold);
	}
	return n;
}

/*! Build a HWRT_Material from an interpolated corner. */
world::HWRT_Material makeMaterial(const RTCorner& c, float emissive, int32_t albedoMapId)
{
	world::HWRT_Material m = {};
	c.normal.normalized().storeUnaligned3(m.normal);
	c.albedo.storeUnaligned3(m.albedo);
	m.emissive = emissive;
	m.albedoMap = albedoMapId;
	m.texCoord[0] = c.texCoord.x;
	m.texCoord[1] = c.texCoord.y;
	return m;
}

/*! Emit a single ray tracing triangle from interpolated corners, generating new vertices. */
void emitClippedTriangle(const RTCorner& a, const RTCorner& b, const RTCorner& c, const uint32_t srcVertices[3], uint32_t baseVertexCount, float emissive, int32_t albedoMapId, RayTracingGeometry& out)
{
	const RTCorner* corners[] = { &a, &b, &c };
	for (const RTCorner* corner : corners)
	{
		const uint32_t index = baseVertexCount + (uint32_t)out.extraPositions.size();
		out.extraPositions.push_back(corner->position);

		RayTracingVertexSource source;
		source.vertices[0] = srcVertices[0];
		source.vertices[1] = srcVertices[1];
		source.vertices[2] = srcVertices[2];
		source.weights[0] = corner->weights[0];
		source.weights[1] = corner->weights[1];
		source.weights[2] = corner->weights[2];
		out.extraVertexSources.push_back(source);

		out.indices.push_back(index);
		out.materials.push_back(makeMaterial(*corner, emissive, albedoMapId));
	}
}

/*! Clip and tessellate a single alpha tested triangle to its coverage mask. */
void clipTriangle(const RTCorner src[3], const uint32_t srcVertices[3], const drawing::Image* image, uint32_t baseVertexCount, float emissive, int32_t albedoMapId, RayTracingGeometry& out)
{
	const int32_t texW = image->getWidth();
	const int32_t texH = image->getHeight();

	// Determine subdivision level from the triangle's texel footprint so a cell is
	// roughly the size of a texel; keep the coverage field ~linear per cell.
	Vector2 uvMin = src[0].texCoord;
	Vector2 uvMax = src[0].texCoord;
	for (int32_t i = 1; i < 3; ++i)
	{
		uvMin.x = std::min(uvMin.x, src[i].texCoord.x);
		uvMin.y = std::min(uvMin.y, src[i].texCoord.y);
		uvMax.x = std::max(uvMax.x, src[i].texCoord.x);
		uvMax.y = std::max(uvMax.y, src[i].texCoord.y);
	}

	const float spanTexels = std::max((uvMax.x - uvMin.x) * texW, (uvMax.y - uvMin.y) * texH);
	int32_t N = (int32_t)std::ceil(spanTexels / c_targetTexelsPerCell);
	N = clamp(N, 1, c_maxSubdivisions);

	const float invN = 1.0f / (float)N;

	// Walk the barycentric lattice, clipping each micro triangle to the coverage.
	for (int32_t i = 0; i < N; ++i)
	{
		for (int32_t j = 0; j < N - i; ++j)
		{
			// Lattice point (a, b) -> weights ( (N-a-b)/N, a/N, b/N ).
			const auto corner = [&](int32_t a, int32_t b) -> RTCorner {
				const float w1 = a * invN;
				const float w2 = b * invN;
				return barycentricCorner(src, 1.0f - w1 - w2, w1, w2, image);
			};

			// Lower ("up") micro triangle.
			{
				const RTCorner tri[3] = { corner(i, j), corner(i + 1, j), corner(i, j + 1) };
				RTCorner poly[4];
				const int32_t nc = clipTriangleToCoverage(tri, c_alphaThreshold, poly);
				for (int32_t k = 1; k + 1 < nc; ++k)
					emitClippedTriangle(poly[0], poly[k], poly[k + 1], srcVertices, baseVertexCount, emissive, albedoMapId, out);
			}

			// Upper ("down") micro triangle.
			if (j < N - i - 1)
			{
				const RTCorner tri[3] = { corner(i + 1, j), corner(i + 1, j + 1), corner(i, j + 1) };
				RTCorner poly[4];
				const int32_t nc = clipTriangleToCoverage(tri, c_alphaThreshold, poly);
				for (int32_t k = 1; k + 1 < nc; ++k)
					emitClippedTriangle(poly[0], poly[k], poly[k + 1], srcVertices, baseVertexCount, emissive, albedoMapId, out);
			}
		}
	}
}

}

void buildRayTracingGeometry(
	const model::Model* model,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	uint32_t baseVertexCount,
	AlignedVector< resource::Id< render::ITexture > >& outAlbedoTextures,
	RayTracingGeometry& outGeometry)
{
	for (const auto& mt : materialTechniqueMap)
	{
		const uint32_t materialId = model->findMaterial(mt.first);
		if (materialId == model::c_InvalidIndex)
			continue;

		const auto& material = model->getMaterial(materialId);

		// Look up index of albedo map, if map doesn't exist add a new reference.
		int32_t albedoMapId = -1;
		if (material.getDiffuseMap().texture.isNotNull())
		{
			const resource::Id< render::ITexture > diffuseId(material.getDiffuseMap().texture);
			const auto it = std::find(outAlbedoTextures.begin(), outAlbedoTextures.end(), diffuseId);
			if (it != outAlbedoTextures.end())
				albedoMapId = (int32_t)std::distance(outAlbedoTextures.begin(), it);
			else
			{
				albedoMapId = (int32_t)outAlbedoTextures.size();
				outAlbedoTextures.push_back(diffuseId);
			}
		}

		const float emissive = material.getEmissive();
		const Vector4 materialColor = material.getColor();

		// Alpha tested surfaces are cut to their diffuse coverage mask, but only
		// when the mask is actually available (with an alpha channel).
		const drawing::Image* alphaImage = nullptr;
		if (material.getBlendOperator() == model::Material::BoAlphaTest)
		{
			const drawing::Image* diffuseImage = material.getDiffuseMap().image;
			if (diffuseImage != nullptr && diffuseImage->getPixelFormat().getAlphaBits() > 0)
				alphaImage = diffuseImage;
		}

		for (const auto& polygon : model->getPolygonsByMaterial(materialId))
		{
			T_ASSERT(polygon.getVertices().size() == 3);

			RTCorner src[3];
			uint32_t srcVertices[3];
			for (int32_t j = 0; j < 3; ++j)
			{
				srcVertices[j] = polygon.getVertex(j);

				const auto& vertex = model->getVertex(srcVertices[j]);
				src[j].position = model->getPosition(vertex.getPosition());
				src[j].normal = (vertex.getNormal() != model::c_InvalidIndex) ? model->getNormal(vertex.getNormal()) : Vector4(0.0f, 1.0f, 0.0f, 0.0f);
				src[j].albedo = (vertex.getColor() != model::c_InvalidIndex) ? model->getColor(vertex.getColor()) : materialColor;
				src[j].texCoord = (vertex.getTexCoord(0) != model::c_InvalidIndex) ? model->getTexCoord(vertex.getTexCoord(0)) : Vector2(0.0f, 0.0f);
				src[j].weights[0] = src[j].weights[1] = src[j].weights[2] = 0.0f;
				src[j].weights[j] = 1.0f;
				src[j].alpha = 1.0f;
			}

			if (alphaImage != nullptr)
			{
				// Cut and tessellate to the coverage mask.
				clipTriangle(src, srcVertices, alphaImage, baseVertexCount, emissive, albedoMapId, outGeometry);
			}
			else
			{
				// Keep the original triangle, referencing the model's own vertices.
				for (int32_t j = 0; j < 3; ++j)
				{
					outGeometry.indices.push_back(polygon.getVertex(j));
					outGeometry.materials.push_back(makeMaterial(src[j], emissive, albedoMapId));
				}
			}
		}
	}
}

}

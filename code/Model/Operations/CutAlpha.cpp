/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Operations/CutAlpha.h"

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Material.h"
#include "Model/Model.h"
#include "Model/Polygon.h"
#include "Model/Vertex.h"

#include <algorithm>
#include <cmath>

namespace traktor::model
{
namespace
{

const float c_alphaThreshold = 0.5f;	   /*!< Alpha coverage threshold; texels with alpha below this are cut away. */
const float c_targetTexelsPerCell = 16.0f; /*!< Target size, in texels, of a tessellation cell along the longest UV edge. */
const int32_t c_maxSubdivisions = 4;	   /*!< Upper bound on triangle subdivision to keep the generated geometry bounded. */

/*! Which optional attributes the source triangle carries. */
struct Attributes
{
	bool normal;
	bool tangent;
	bool binormal;
	bool color;
};

/*! Interpolated triangle corner used while clipping. */
struct Corner
{
	Vector4 position;
	Vector4 normal;
	Vector4 tangent;
	Vector4 binormal;
	Vector4 color;
	Vector2 texCoord;
	float weights[3]; //!< Barycentric weights within the source triangle.
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
Corner barycentricCorner(const Corner src[3], float w0, float w1, float w2, const drawing::Image* image)
{
	Corner c;
	c.position = src[0].position * Scalar(w0) + src[1].position * Scalar(w1) + src[2].position * Scalar(w2);
	c.normal = src[0].normal * Scalar(w0) + src[1].normal * Scalar(w1) + src[2].normal * Scalar(w2);
	c.tangent = src[0].tangent * Scalar(w0) + src[1].tangent * Scalar(w1) + src[2].tangent * Scalar(w2);
	c.binormal = src[0].binormal * Scalar(w0) + src[1].binormal * Scalar(w1) + src[2].binormal * Scalar(w2);
	c.color = src[0].color * Scalar(w0) + src[1].color * Scalar(w1) + src[2].color * Scalar(w2);
	c.texCoord = src[0].texCoord * w0 + src[1].texCoord * w1 + src[2].texCoord * w2;
	c.weights[0] = w0;
	c.weights[1] = w1;
	c.weights[2] = w2;
	c.alpha = sampleAlpha(image, c.texCoord);
	return c;
}

/*! Interpolate a corner where the coverage field crosses the threshold. */
Corner intersectCorner(const Corner& a, const Corner& b, float threshold)
{
	const float d = b.alpha - a.alpha;
	const float t = (std::abs(d) > 1e-8f) ? (threshold - a.alpha) / d : 0.0f;
	Corner c;
	c.position = lerp(a.position, b.position, Scalar(t));
	c.normal = lerp(a.normal, b.normal, Scalar(t));
	c.tangent = lerp(a.tangent, b.tangent, Scalar(t));
	c.binormal = lerp(a.binormal, b.binormal, Scalar(t));
	c.color = lerp(a.color, b.color, Scalar(t));
	c.texCoord = lerp(a.texCoord, b.texCoord, t);
	c.weights[0] = a.weights[0] + (b.weights[0] - a.weights[0]) * t;
	c.weights[1] = a.weights[1] + (b.weights[1] - a.weights[1]) * t;
	c.weights[2] = a.weights[2] + (b.weights[2] - a.weights[2]) * t;
	c.alpha = threshold;
	return c;
}

/*! Clip a triangle against the covered half-space (alpha >= threshold). Returns the
 *  covered convex polygon (0, 3 or 4 corners). */
int32_t clipTriangleToCoverage(const Corner in[3], float threshold, Corner out[4])
{
	int32_t n = 0;
	for (int32_t i = 0; i < 3; ++i)
	{
		const Corner& cur = in[i];
		const Corner& nxt = in[(i + 1) % 3];
		const bool curIn = (cur.alpha >= threshold);
		const bool nxtIn = (nxt.alpha >= threshold);
		if (curIn)
			out[n++] = cur;
		if (curIn != nxtIn)
			out[n++] = intersectCorner(cur, nxt, threshold);
	}
	return n;
}

/*! Emit one clipped triangle as a model polygon, generating new vertices whose attributes
 *  are the barycentric blend of the source triangle (joint influences included). */
void emitTriangle(
	Model& model,
	const Corner& a,
	const Corner& b,
	const Corner& c,
	const Attributes& attributes,
	const SmallMap< uint32_t, float > sourceInfluences[3],
	uint32_t materialId,
	AlignedVector< Polygon >& outPolygons)
{
	Polygon polygon;
	polygon.setMaterial(materialId);

	const Corner* corners[] = { &a, &b, &c };
	for (const Corner* corner : corners)
	{
		Vertex vertex;
		vertex.setPosition(model.addUniquePosition(corner->position));
		if (attributes.normal)
			vertex.setNormal(model.addUniqueNormal(corner->normal.normalized()));
		if (attributes.tangent)
			vertex.setTangent(model.addUniqueNormal(corner->tangent.normalized()));
		if (attributes.binormal)
			vertex.setBinormal(model.addUniqueNormal(corner->binormal.normalized()));
		if (attributes.color)
			vertex.setColor(model.addUniqueColor(corner->color));
		vertex.setTexCoord(0, model.addUniqueTexCoord(corner->texCoord));

		// Skin the generated vertex by blending the source triangle's influences.
		for (int32_t k = 0; k < 3; ++k)
		{
			const float w = corner->weights[k];
			if (w <= 0.0f)
				continue;
			for (const auto& influence : sourceInfluences[k])
				vertex.setJointInfluence(influence.first, vertex.getJointInfluence(influence.first) + w * influence.second);
		}

		polygon.addVertex(model.addUniqueVertex(vertex));
	}

	outPolygons.push_back(polygon);
}

/*! Clip and tessellate a single alpha tested triangle to its coverage mask. */
void clipTriangle(
	Model& model,
	const Corner src[3],
	const Attributes& attributes,
	const SmallMap< uint32_t, float > sourceInfluences[3],
	const drawing::Image* image,
	uint32_t materialId,
	AlignedVector< Polygon >& outPolygons)
{
	const int32_t texW = image->getWidth();
	const int32_t texH = image->getHeight();

	// Determine subdivision level from the triangle's texel footprint so a cell is roughly the
	// size of a texel; keeps the coverage field ~linear per cell.
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
			const auto corner = [&](int32_t a, int32_t b) -> Corner {
				const float w1 = a * invN;
				const float w2 = b * invN;
				return barycentricCorner(src, 1.0f - w1 - w2, w1, w2, image);
			};

			// Lower ("up") micro triangle.
			{
				const Corner tri[3] = { corner(i, j), corner(i + 1, j), corner(i, j + 1) };
				Corner poly[4];
				const int32_t nc = clipTriangleToCoverage(tri, c_alphaThreshold, poly);
				for (int32_t k = 1; k + 1 < nc; ++k)
					emitTriangle(model, poly[0], poly[k], poly[k + 1], attributes, sourceInfluences, materialId, outPolygons);
			}

			// Upper ("down") micro triangle.
			if (j < N - i - 1)
			{
				const Corner tri[3] = { corner(i + 1, j), corner(i + 1, j + 1), corner(i, j + 1) };
				Corner poly[4];
				const int32_t nc = clipTriangleToCoverage(tri, c_alphaThreshold, poly);
				for (int32_t k = 1; k + 1 < nc; ++k)
					emitTriangle(model, poly[0], poly[k], poly[k + 1], attributes, sourceInfluences, materialId, outPolygons);
			}
		}
	}
}

/*! Coverage mask of an alpha tested material, or null if the material is not cut. */
const drawing::Image* getCoverageMask(const Material& material)
{
	if (material.getBlendOperator() != Material::BoAlphaTest)
		return nullptr;
	const drawing::Image* diffuseImage = material.getDiffuseMap().image;
	if (diffuseImage == nullptr || diffuseImage->getPixelFormat().getAlphaBits() <= 0)
		return nullptr;
	return diffuseImage;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CutAlpha", CutAlpha, IModelOperation)

bool CutAlpha::apply(Model& model) const
{
	const AlignedVector< Polygon >& polygons = model.getPolygons();

	AlignedVector< Polygon > outPolygons;
	outPolygons.reserve(polygons.size());

	for (const Polygon& polygon : polygons)
	{
		const uint32_t materialId = polygon.getMaterial();
		const drawing::Image* mask = (materialId != c_InvalidIndex) ? getCoverageMask(model.getMaterial(materialId)) : nullptr;

		// Keep anything we cannot (or need not) cut: non alpha tested surfaces, non triangles,
		// or triangles without texture coordinates to sample the mask with.
		bool cut = (mask != nullptr && polygon.getVertices().size() == 3);
		if (cut)
		{
			for (uint32_t j = 0; j < 3; ++j)
				if (model.getVertex(polygon.getVertex(j)).getTexCoord(0) == c_InvalidIndex)
					cut = false;
		}
		if (!cut)
		{
			outPolygons.push_back(polygon);
			continue;
		}

		// Gather the source triangle. Optional attributes are taken per-triangle from the first
		// corner; joint influences are copied up-front so appending vertices can't dangle them.
		const Vertex& v0 = model.getVertex(polygon.getVertex(0));
		Attributes attributes;
		attributes.normal = (v0.getNormal() != c_InvalidIndex);
		attributes.tangent = (v0.getTangent() != c_InvalidIndex);
		attributes.binormal = (v0.getBinormal() != c_InvalidIndex);
		attributes.color = (v0.getColor() != c_InvalidIndex);

		Corner src[3];
		SmallMap< uint32_t, float > sourceInfluences[3];
		for (uint32_t j = 0; j < 3; ++j)
		{
			const Vertex& vertex = model.getVertex(polygon.getVertex(j));
			src[j].position = model.getPosition(vertex.getPosition());
			src[j].normal = attributes.normal ? model.getNormal(vertex.getNormal()) : Vector4::zero();
			src[j].tangent = attributes.tangent ? model.getNormal(vertex.getTangent()) : Vector4::zero();
			src[j].binormal = attributes.binormal ? model.getNormal(vertex.getBinormal()) : Vector4::zero();
			src[j].color = attributes.color ? model.getColor(vertex.getColor()) : Vector4::zero();
			src[j].texCoord = model.getTexCoord(vertex.getTexCoord(0));
			src[j].weights[0] = src[j].weights[1] = src[j].weights[2] = 0.0f;
			src[j].weights[j] = 1.0f;
			src[j].alpha = 1.0f;
			sourceInfluences[j] = vertex.getJointInfluences();
		}

		clipTriangle(model, src, attributes, sourceInfluences, mask, materialId, outPolygons);
	}

	model.setPolygons(outPolygons);
	return true;
}

}

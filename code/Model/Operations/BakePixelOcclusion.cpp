#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Triangle.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Model.h"
#include "Model/Operations/BakePixelOcclusion.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct BakePixelVisitor
{
	RandomGeometry& random;
	const SahTree& sah;
	Vector4 P[3];
	Vector4 N[3];
	drawing::Image& outImage;
	SahTree::QueryCache cache;

	BakePixelVisitor(
		RandomGeometry& _random,
		const SahTree& _sah,
		const Vector4 _P[3],
		const Vector4 _N[3],
		drawing::Image& _outImage
	)
	:	random(_random)
	,	sah(_sah)
	,	outImage(_outImage)
	{
		for (int i = 0; i < 3; ++i)
		{
			P[i] = _P[i];
			N[i] = _N[i];
		}
	}

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		const uint32_t c_occlusionRayCount = 64;
		const Scalar c_occlusionRaySpread(0.75f);
		const static Scalar c_occlusionRayBias(0.1f);

		Vector4 position = (P[0] * Scalar(alpha) + P[1] * Scalar(beta) + P[2] * Scalar(gamma)).xyz1();
		Vector4 normal = (N[0] * Scalar(alpha) + N[1] * Scalar(beta) + N[2] * Scalar(gamma)).xyz0();

		uint32_t occluded = 0;
		for (uint32_t j = 0; j < c_occlusionRayCount; ++j)
		{
			Vector4 rayDirection = lerp(normal, random.nextHemi(normal), c_occlusionRaySpread).normalized().xyz0();
			Vector4 rayOrigin = (position + normal * c_occlusionRayBias).xyz1();
			if (sah.queryAnyIntersection(rayOrigin, rayDirection, 0.0f, cache))
				occluded++;
		}

		float occf = 1.0f - float(occluded) / c_occlusionRayCount;
		outImage.setPixel(x, y, Color4f(occf, occf, occf, occf));
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.BakePixelOcclusion", BakePixelOcclusion, IModelOperation)

BakePixelOcclusion::BakePixelOcclusion(
	drawing::Image* occlusionImage,
	int32_t rayCount,
	float raySpread,
	float rayBias
)
:	m_occlusionImage(occlusionImage)
,	m_rayCount(rayCount)
,	m_raySpread(raySpread)
,	m_rayBias(rayBias)
{
}

bool BakePixelOcclusion::apply(Model& model) const
{
	RandomGeometry rnd;

	const std::vector< Polygon >& polygons = model.getPolygons();
	std::vector< Vertex > vertices = model.getVertices();

	AlignedVector< Vector4 > colors = model.getColors();
	model.clear(Model::CfColors);

	AlignedVector< Winding3 > windings(polygons.size());
	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		Winding3& w = windings[i];
		const std::vector< uint32_t >& vertexIndices = polygons[i].getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertexIndices.begin(); j != vertexIndices.end(); ++j)
		{
			const Vertex& polyVertex = model.getVertex(*j);
			const Vector4& polyVertexPosition = model.getPosition(polyVertex.getPosition());
			w.push(polyVertexPosition);
		}
	}

	// Build acceleration tree.
	SahTree sah;
	sah.build(windings);

	// Create occlusion image.
	m_occlusionImage->clear(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	// Trace each polygon in UV space.
	Vector2 dim(float(m_occlusionImage->getWidth()), float(m_occlusionImage->getHeight()));
	Vector2 uv[3];
	Vector4 P[3], N[3];

	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		const Vertex& v0 = vertices[polygons[i].getVertex(0)];
		const Vertex& v1 = vertices[polygons[i].getVertex(1)];
		const Vertex& v2 = vertices[polygons[i].getVertex(2)];

		int32_t tc0 = v0.getTexCoord(0);
		int32_t tc1 = v1.getTexCoord(0);
		int32_t tc2 = v2.getTexCoord(0);

		if (tc0 == c_InvalidIndex || tc1 == c_InvalidIndex || tc2 == c_InvalidIndex)
			continue;

		uv[0] = model.getTexCoord(tc0) * dim;
		uv[1] = model.getTexCoord(tc1) * dim;
		uv[2] = model.getTexCoord(tc2) * dim;

		P[0] = model.getPosition(v0.getPosition());
		P[1] = model.getPosition(v1.getPosition());
		P[2] = model.getPosition(v2.getPosition());

		int32_t n0 = v0.getNormal();
		int32_t n1 = v1.getNormal();
		int32_t n2 = v2.getNormal();

		if (n0 == c_InvalidIndex || n1 == c_InvalidIndex || n2 == c_InvalidIndex)
			continue;

		N[0] = model.getNormal(n0);
		N[1] = model.getNormal(n1);
		N[2] = model.getNormal(n2);

		BakePixelVisitor visitor(rnd, sah, P, N, *m_occlusionImage);
		triangle(uv[0], uv[1], uv[2], visitor);
	}

	return true;
}

	}
}

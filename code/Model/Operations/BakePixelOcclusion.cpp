/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Triangle.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/DilateFilter.h"
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
	RandomGeometry random;
	SahTree sah;
	uint32_t rayCount;
	Scalar raySpread;
	Scalar rayBias;
	Vector4 P[3];
	Vector4 N[3];
	drawing::Image* outImage;
	SahTree::QueryCache cache;

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		Vector4 position = (P[0] * Scalar(alpha) + P[1] * Scalar(beta) + P[2] * Scalar(gamma)).xyz1();
		Vector4 normal = (N[0] * Scalar(alpha) + N[1] * Scalar(beta) + N[2] * Scalar(gamma)).xyz0();

		uint32_t occluded = 0;
		for (uint32_t j = 0; j < rayCount; ++j)
		{
			Vector4 rayDirection = lerp(normal, random.nextHemi(normal), raySpread).normalized();
			Vector4 rayOrigin = position + normal * rayBias;
			if (sah.queryAnyIntersection(rayOrigin, rayDirection, 100.0f, cache))
				occluded++;
		}

		float occf = 1.0f - float(occluded) / rayCount;
		outImage->setPixel(x, y, Color4f(occf, occf, occf, 1.0f));
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

	BakePixelVisitor visitor;
	visitor.rayCount = m_rayCount;
	visitor.raySpread = Scalar(m_raySpread);
	visitor.rayBias = Scalar(m_rayBias);
	visitor.outImage = m_occlusionImage;

	// Build acceleration tree.
	visitor.sah.build(windings);

	// Create occlusion image.
	m_occlusionImage->clear(Color4f(1.0f, 1.0f, 1.0f, 0.0f));

	// Trace each polygon in UV space.
	Vector2 dim(float(m_occlusionImage->getWidth()), float(m_occlusionImage->getHeight()));
	Vector2 uv[3];

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

		visitor.P[0] = model.getPosition(v0.getPosition());
		visitor.P[1] = model.getPosition(v1.getPosition());
		visitor.P[2] = model.getPosition(v2.getPosition());

		int32_t n0 = v0.getNormal();
		int32_t n1 = v1.getNormal();
		int32_t n2 = v2.getNormal();

		if (n0 == c_InvalidIndex || n1 == c_InvalidIndex || n2 == c_InvalidIndex)
			continue;

		visitor.N[0] = model.getNormal(n0);
		visitor.N[1] = model.getNormal(n1);
		visitor.N[2] = model.getNormal(n2);

		Vector2 l = uv[1] - uv[0];
		Vector2 r = uv[2] - uv[0];
		if (l.x * r.y - r.x * l.y > 0.0f)
		{
			std::swap(uv[0], uv[2]);
			std::swap(visitor.P[0], visitor.P[2]);
			std::swap(visitor.N[0], visitor.N[2]);
		}

		triangle(uv[0], uv[1], uv[2], visitor);
	}

	drawing::DilateFilter dilateFilter(4);
	m_occlusionImage->apply(&dilateFilter);
	m_occlusionImage->apply(drawing::ConvolutionFilter::createGaussianBlur5());
	
	return true;
}

	}
}

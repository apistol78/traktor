#include <MaxRectsBinPack.h>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Const.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Winding2.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/UnwrapUV.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const float c_texelsPerUnit = 10.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.UnwrapUV", UnwrapUV, IModelOperation)

UnwrapUV::UnwrapUV(int32_t channel, float margin)
:	m_channel(channel)
,	m_margin(margin)
{
}

bool UnwrapUV::apply(Model& model) const
{
	std::vector< Polygon > originalPolygons = model.getPolygons();
	AlignedVector< Winding2 > wuvs;
	std::vector< int32_t > majors;

	// Create windings, determine projection plane and calculate initial projected UV set.
	for (uint32_t i = 0; i < originalPolygons.size(); ++i)
	{
		const Polygon& polygon = originalPolygons[i];

		const std::vector< uint32_t >& vertexIds = polygon.getVertices();
		if (vertexIds.size() < 3)
			return false;

		Winding3 w;
		for (uint32_t j = 0; j < vertexIds.size(); ++j)
		{
			const Vector4& position = model.getVertexPosition(vertexIds[j]);
			w.push(position);
		}

		Plane wp;
		if (!w.getPlane(wp))
			return false;

		int32_t major = majorAxis3(wp.normal());

		if (wp.normal().get(major) > 0.0f)
			majors.push_back((major + 1));
		else
			majors.push_back(-(major + 1));

		Winding2 wuv;
		for (uint32_t j = 0; j < w.size(); ++j)
		{
			switch (major)
			{
			case 0:	// X-axis
				{
					float u = w[j].y();
					float v = w[j].z();
					if (wp.normal().x() < 0.0f)
						wuv.points.push_back(Vector2(u, v) * c_texelsPerUnit);
					else
						wuv.points.push_back(Vector2(u, -v) * c_texelsPerUnit);
				}
				break;

			case 1:	// Y-axis
				{
					float u = w[j].x();
					float v = w[j].z();
					if (wp.normal().y() > 0.0f)
						wuv.points.push_back(Vector2(u, v) * c_texelsPerUnit);
					else
						wuv.points.push_back(Vector2(u, -v) * c_texelsPerUnit);
				}
				break;

			case 2:	// Z-axis
				{
					float u = w[j].x();
					float v = w[j].y();
					if (wp.normal().z() < 0.0f)
						wuv.points.push_back(Vector2(u, v) * c_texelsPerUnit);
					else
						wuv.points.push_back(Vector2(u, -v) * c_texelsPerUnit);
				}
				break;

			default:
				return false;
			}
		}
		wuvs.push_back(wuv);
	}

	T_FATAL_ASSERT (wuvs.size() == originalPolygons.size());

	// Put neighbor polygons in same groups
	std::vector< uint32_t > groups(originalPolygons.size(), c_InvalidIndex);
	std::vector< uint32_t > sharedEdges;
	uint32_t lastGroupId = 0;

	ModelAdjacency adjacency(&model, ModelAdjacency::MdByPosition);
	for (uint32_t i = 0; i < originalPolygons.size(); ++i)
	{
		if (groups[i] == c_InvalidIndex)
			groups[i] = lastGroupId++;

		for (uint32_t edge = 0; edge < originalPolygons[i].getVertexCount(); ++edge)
		{
			adjacency.getSharedEdges(i, edge, sharedEdges);
			for (std::vector< uint32_t >::const_iterator it = sharedEdges.begin(); it != sharedEdges.end(); ++it)
			{
				uint32_t sharedPolygon = adjacency.getPolygon(*it);
				T_FATAL_ASSERT (sharedPolygon != i);

				if (majors[sharedPolygon] == majors[i] && groups[sharedPolygon] == c_InvalidIndex)
					groups[sharedPolygon] = groups[i];
			}
		}
	}

	T_FATAL_ASSERT (groups.size() == originalPolygons.size());

	// For each group calculate bounding boxes.
	AlignedVector< Aabb2 > aabbuvs;
	float totalTexelArea = 0.0f;

	for (uint32_t i = 0; i < lastGroupId; ++i)
	{
		Aabb2 aabbuv;

		for (uint32_t j = 0; j < originalPolygons.size(); ++j)
		{
			if (groups[j] == i)
			{
				for (uint32_t k = 0; k < wuvs[j].points.size(); ++k)
					aabbuv.contain(wuvs[j].points[k]);
			}
		}

		T_FATAL_ASSERT (!aabbuv.empty());

		aabbuv.mn -= Vector2(m_margin, m_margin);
		aabbuv.mx += Vector2(m_margin, m_margin);

		Vector2 size = (aabbuv.mx - aabbuv.mn);
		totalTexelArea += size.x * size.y;

		aabbuvs.push_back(aabbuv);
	}

	T_FATAL_ASSERT (aabbuvs.size() == lastGroupId);

	int32_t size = std::max(int32_t(std::sqrt(totalTexelArea)), 16);
	
	// Pack each group into a separate rectangle.
	rbp::MaxRectsBinPack binPack;
	binPack.Init(size, size);

	std::vector< rbp::Rect > rects;
	for (uint32_t i = 0; i < aabbuvs.size(); )
	{
		int32_t width = int32_t((aabbuvs[i].mx - aabbuvs[i].mn).x + 0.5f);
		int32_t height = int32_t((aabbuvs[i].mx - aabbuvs[i].mn).y + 0.5f);
		T_FATAL_ASSERT (width > 0);
		T_FATAL_ASSERT (height > 0);

		rbp::Rect packedRect = binPack.Insert(width, height, rbp::MaxRectsBinPack::RectBestAreaFit);
		if (packedRect.height <= 0)
		{
			i = 0;
			size += size / 2;
			binPack.Init(size, size);
			rects.resize(0);
			continue;
		}

		rects.push_back(packedRect);
		++i;
	}

	// Update UV for each polygon.
	std::vector< Vertex > originalVertices = model.getVertices();
	model.clear(Model::CfPolygons | Model::CfVertices);

	for (uint32_t i = 0; i < originalPolygons.size(); ++i)
	{
		uint32_t g = groups[i];
		const rbp::Rect& packedRect = rects[g];

		int32_t width = int32_t((aabbuvs[g].mx - aabbuvs[g].mn).x + 0.5f);
		int32_t height = int32_t((aabbuvs[g].mx - aabbuvs[g].mn).y + 0.5f);

		bool flipped = false;
		if (packedRect.width > packedRect.height)
		{
			if (width < height)
				flipped = true;
		}
		else if (packedRect.width < packedRect.height)
		{
			if (width > height)
				flipped = true;
		}

		for (uint32_t j = 0; j < wuvs[i].points.size(); ++j)
		{
			Vertex vertex = originalVertices[originalPolygons[i].getVertex(j)];

			Vector2 uv = (wuvs[i].points[j] - aabbuvs[g].mn) / (aabbuvs[g].mx - aabbuvs[g].mn);
			T_FATAL_ASSERT (uv.x >= -FUZZY_EPSILON && uv.x <= 1.0f + FUZZY_EPSILON);
			T_FATAL_ASSERT (uv.y >= -FUZZY_EPSILON && uv.y <= 1.0f + FUZZY_EPSILON);

			if (flipped)
			{
				std::swap(uv.x, uv.y);
				uv.x = 1.0f - uv.x;
			}

			uv.x = (uv.x * packedRect.width + packedRect.x) / size;
			uv.y = (uv.y * packedRect.height + packedRect.y) / size;
			vertex.setTexCoord(m_channel, model.addUniqueTexCoord(uv));

			originalPolygons[i].setVertex(j, model.addUniqueVertex(vertex));
		}
	}

	model.setPolygons(originalPolygons);
	return true;
}

	}
}

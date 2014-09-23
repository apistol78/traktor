#include <MaxRectsBinPack.h>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Const.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Winding2.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
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
	AlignedVector< Aabb2 > aabbuvs;
	float totalTexelArea = 0.0f;

	for (uint32_t i = 0; i < originalPolygons.size(); ++i)
	{
		const Polygon& polygon = originalPolygons[i];

		Winding3 w;

		const std::vector< uint32_t >& vertexIds = polygon.getVertices();
		for (uint32_t j = 0; j < vertexIds.size(); ++j)
		{
			const Vertex& vertex = model.getVertex(vertexIds[j]);
			uint32_t positionId = vertex.getPosition();
			const Vector4& position = model.getPosition(positionId);
			w.push(position);
		}

		Plane wp;
		if (!w.getPlane(wp))
			return false;

		int32_t major = majorAxis3(wp.normal());

		Winding2 wuv;
		Aabb2 aabbuv;

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

			aabbuv.contain(wuv.points[j]);
		}

		wuvs.push_back(wuv);
		aabbuvs.push_back(aabbuv);

		aabbuvs[i].mn -= Vector2(m_margin, m_margin);
		aabbuvs[i].mx += Vector2(m_margin, m_margin);

		Vector2 size = (aabbuvs[i].mx - aabbuvs[i].mn);
		totalTexelArea += size.x * size.y;
	}

	int32_t size = nearestLog2(int32_t(std::sqrt(totalTexelArea)));
	
	std::vector< Vertex > originalVertices = model.getVertices();
	model.clear(Model::CfPolygons | Model::CfVertices);

	rbp::MaxRectsBinPack binPack;
	binPack.Init(size, size);

	for (uint32_t i = 0; i < wuvs.size(); )
	{
		int32_t width = int32_t((aabbuvs[i].mx - aabbuvs[i].mn).x + 0.5f);
		int32_t height = int32_t((aabbuvs[i].mx - aabbuvs[i].mn).y + 0.5f);
		T_FATAL_ASSERT (width > 0);
		T_FATAL_ASSERT (height > 0);

		rbp::Rect packedRect = binPack.Insert(width, height, rbp::MaxRectsBinPack::RectBestAreaFit);
		if (packedRect.height <= 0)
		{
			i = 0;
			size *= 2;
			binPack.Init(size, size);
			model.clear(Model::CfPolygons | Model::CfVertices);
			continue;
		}

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

			Vector2 uv = (wuvs[i].points[j] - aabbuvs[i].mn) / (aabbuvs[i].mx - aabbuvs[i].mn);
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

		++i;
	}

	model.setPolygons(originalPolygons);
	return true;
}

	}
}

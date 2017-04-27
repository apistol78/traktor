/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#define STB_RECT_PACK_IMPLEMENTATION
#include <cmath>
#include <stb_rect_pack.h>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Const.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Winding2.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/AutoPtr.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/UnwrapUV.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.UnwrapUV", UnwrapUV, IModelOperation)

UnwrapUV::UnwrapUV(int32_t channel, float uvPerUnit, int32_t size, int32_t margin)
:	m_channel(channel)
,	m_uvPerUnit(uvPerUnit)
,	m_size(size)
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
						wuv.points.push_back(Vector2(u, v) * m_uvPerUnit);
					else
						wuv.points.push_back(Vector2(u, -v) * m_uvPerUnit);
				}
				break;

			case 1:	// Y-axis
				{
					float u = w[j].x();
					float v = w[j].z();
					if (wp.normal().y() > 0.0f)
						wuv.points.push_back(Vector2(u, v) * m_uvPerUnit);
					else
						wuv.points.push_back(Vector2(u, -v) * m_uvPerUnit);
				}
				break;

			case 2:	// Z-axis
				{
					float u = w[j].x();
					float v = w[j].y();
					if (wp.normal().z() < 0.0f)
						wuv.points.push_back(Vector2(u, v) * m_uvPerUnit);
					else
						wuv.points.push_back(Vector2(u, -v) * m_uvPerUnit);
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

	// Rotate each group so longest edge of it's hull is horizontal.
	for (uint32_t i = 0; i < lastGroupId; ++i)
	{
		AlignedVector< Vector2 > pnts;
		for (uint32_t j = 0; j < originalPolygons.size(); ++j)
		{
			if (groups[j] == i)
			{
				for (uint32_t k = 0; k < wuvs[j].points.size(); ++k)
					pnts.push_back(wuvs[j].points[k]);
			}
		}

		T_FATAL_ASSERT (pnts.size() >= 3);

		Winding2 hull = Winding2::convexHull(pnts);

		float mln = 0.0f;
		float angle = 0.0f;

		for (uint32_t j = 0; j < hull.points.size(); ++j)
		{
			uint32_t k = (j + 1) % hull.points.size();
			Vector2 edge = hull.points[k] - hull.points[j];
			float ln = edge.length();
			if (ln > mln)
			{
				mln = ln;
				angle = std::atan2(edge.y, edge.x);
			}
		}

		for (uint32_t j = 0; j < originalPolygons.size(); ++j)
		{
			if (groups[j] == i)
			{
				for (uint32_t k = 0; k < wuvs[j].points.size(); ++k)
					wuvs[j].points[k] = rotate(-angle) * wuvs[j].points[k];
			}
		}
	}

	// Quantize all UVs to be center of texel.
	for (uint32_t i = 0; i < originalPolygons.size(); ++i)
	{
		for (uint32_t j = 0; j < wuvs[i].points.size(); ++j)
		{
			wuvs[i].points[j].x = std::floor(wuvs[i].points[j].x * m_size) / m_size;
			wuvs[i].points[j].y = std::floor(wuvs[i].points[j].y * m_size) / m_size;
		}
	}

	// For each group calculate bounding boxes.
	AlignedVector< Aabb2 > aabbuvs(lastGroupId);
	for (uint32_t i = 0; i < lastGroupId; ++i)
	{
		Aabb2& aabbuv = aabbuvs[i];
		for (uint32_t j = 0; j < originalPolygons.size(); ++j)
		{
			if (groups[j] == i)
			{
				for (uint32_t k = 0; k < wuvs[j].points.size(); ++k)
					aabbuv.contain(wuvs[j].points[k]);
			}
		}
		T_FATAL_ASSERT (!aabbuv.empty());
	}
	
	// Pack each group into a separate rectangle.
	AutoPtr< stbrp_context > packer(new stbrp_context());
	AutoArrayPtr< stbrp_node > nodes(new stbrp_node [m_size]);

	stbrp_setup_allow_out_of_mem(packer.ptr(), 1);
	stbrp_init_target(packer.ptr(), m_size, m_size, nodes.ptr(), m_size);

	std::vector< stbrp_rect > rects;
	for (uint32_t i = 0; i < aabbuvs.size(); )
	{
		int32_t width = int32_t((aabbuvs[i].mx - aabbuvs[i].mn).x + 0.5f);
		int32_t height = int32_t((aabbuvs[i].mx - aabbuvs[i].mn).y + 0.5f);

		stbrp_rect r = { 0 };
		r.w = width + m_margin * 2;
		r.h = height + m_margin * 2;
		stbrp_pack_rects(packer.ptr(), &r, 1);
		if (!r.was_packed)
			return false;

		rects.push_back(r);
		++i;
	}

	// Update UV for each polygon.
	std::vector< Vertex > originalVertices = model.getVertices();
	model.clear(Model::CfPolygons | Model::CfVertices);

	for (uint32_t i = 0; i < originalPolygons.size(); ++i)
	{
		uint32_t g = groups[i];
		const stbrp_rect& packedRect = rects[g];
		for (uint32_t j = 0; j < wuvs[i].points.size(); ++j)
		{
			Vertex vertex = originalVertices[originalPolygons[i].getVertex(j)];

			Vector2 uv = (wuvs[i].points[j] - aabbuvs[g].mn) / (aabbuvs[g].mx - aabbuvs[g].mn);
			T_FATAL_ASSERT (uv.x >= -FUZZY_EPSILON && uv.x <= 1.0f + FUZZY_EPSILON);
			T_FATAL_ASSERT (uv.y >= -FUZZY_EPSILON && uv.y <= 1.0f + FUZZY_EPSILON);

			uv.x = (uv.x * (packedRect.w - m_margin * 2) + packedRect.x + m_margin) / m_size;
			uv.y = (uv.y * (packedRect.h - m_margin * 2) + packedRect.y + m_margin) / m_size;

			vertex.setTexCoord(m_channel, model.addUniqueTexCoord(uv));
			originalPolygons[i].setVertex(j, model.addUniqueVertex(vertex));
		}
	}

	model.setPolygons(originalPolygons);
	return true;
}

	}
}

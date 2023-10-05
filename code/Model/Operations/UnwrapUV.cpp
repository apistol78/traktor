/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include <xatlas.h>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Model/Model.h"
#include "Model/Operations/UnwrapUV.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.UnwrapUV", UnwrapUV, IModelOperation)

UnwrapUV::UnwrapUV(int32_t channel, uint32_t textureSize)
:	m_channel(channel)
,	m_textureSize(textureSize)
{
}

bool UnwrapUV::apply(Model& model) const
{
	xatlas::Atlas* atlas = xatlas::Create();

	AlignedVector< float > vertices;
	AlignedVector< float > normals;
	AlignedVector< float > texCoords;
	AlignedVector< uint32_t > indices;
	SmallMap< std::tuple< uint32_t, uint32_t, uint32_t >, uint32_t > vertexToPolygon;

	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		const Vertex& vertex = model.getVertex(i);

		const Vector4& p = model.getPosition(vertex.getPosition());
		vertices.push_back(p.x());
		vertices.push_back(p.y());
		vertices.push_back(p.z());

		if (vertex.getNormal() != c_InvalidIndex)
		{
			const Vector4& n = model.getNormal(vertex.getNormal());
			normals.push_back(n.x());
			normals.push_back(n.y());
			normals.push_back(n.z());
		}

		if (vertex.getTexCoord(m_channel) != c_InvalidIndex)
		{
			const Vector2& tc = model.getTexCoord(vertex.getTexCoord(m_channel));
			texCoords.push_back(tc.x);
			texCoords.push_back(tc.y);
		}
	}

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const Polygon& polygon = model.getPolygon(i);
		if (polygon.getVertexCount() != 3)
			continue;

		indices.push_back(polygon.getVertex(0));
		indices.push_back(polygon.getVertex(1));
		indices.push_back(polygon.getVertex(2));

		const std::tuple< uint32_t, uint32_t, uint32_t > key = {
			polygon.getVertex(0),
			polygon.getVertex(1),
			polygon.getVertex(02)
		};

		T_ASSERT(vertexToPolygon.find(key) == vertexToPolygon.end());
		vertexToPolygon[key] = i;
	}

	xatlas::MeshDecl meshDecl;
	meshDecl.vertexCount = (uint32_t)vertices.size() / 3;
	meshDecl.vertexPositionData = vertices.c_ptr();
	meshDecl.vertexPositionStride = sizeof(float) * 3;
	if (!normals.empty())
	{
		meshDecl.vertexNormalData = normals.c_ptr();
		meshDecl.vertexNormalStride = sizeof(float) * 3;
	}
	if (!texCoords.empty())
	{
		meshDecl.vertexUvData = texCoords.c_ptr();
		meshDecl.vertexUvStride = sizeof(float) * 2;
	}
	meshDecl.indexCount = (uint32_t)indices.size();
	meshDecl.indexData = indices.c_ptr();
	meshDecl.indexFormat = xatlas::IndexFormat::UInt32;

	xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl);
	if (error != xatlas::AddMeshError::Success) 
	{
		xatlas::Destroy(atlas);
		return false;
	}

	xatlas::ChartOptions co;

	xatlas::PackOptions po;
	po.blockAlign = true;
	po.padding = 4;
	po.resolution = m_textureSize;

	xatlas::Generate(atlas, co, po);

	if (atlas->width == 0 || atlas->height == 0)
	{
		xatlas::Destroy(atlas);
		return false;
	}

	const AlignedVector< Vertex > originalVertices = model.getVertices();
	const AlignedVector< Polygon > originalPolygons = model.getPolygons();

	model.clear(Model::CfVertices | Model::CfPolygons);

	for (uint32_t i = 0; i < atlas->meshCount; ++i)
	{
		const xatlas::Mesh& mesh = atlas->meshes[i];
		for (uint32_t j = 0; j < mesh.indexCount; j += 3)
		{
			const xatlas::Vertex& v0 = mesh.vertexArray[mesh.indexArray[j + 0]];
			const xatlas::Vertex& v1 = mesh.vertexArray[mesh.indexArray[j + 1]];
			const xatlas::Vertex& v2 = mesh.vertexArray[mesh.indexArray[j + 2]];

			Polygon polygon = originalPolygons[vertexToPolygon[{ v0.xref, v1.xref, v2.xref }]];
			T_ASSERT(polygon.getVertexCount() == 3);

			for (int32_t k = 0; k < 3; ++k)
			{
				const int32_t index = mesh.indexArray[j + k];
				const xatlas::Vertex& v = mesh.vertexArray[index];

				Vector2 uv(v.uv[0], v.uv[1]);
				uv.x /= (float)atlas->width;
				uv.y /= (float)atlas->height;

				Vertex vx = originalVertices[v.xref];
				vx.setTexCoord(m_channel, model.addUniqueTexCoord(uv));
				polygon.setVertex(k, model.addUniqueVertex(vx));
			}

			model.addPolygon(polygon);
		}
	}

	xatlas::Destroy(atlas);
	return true;
}

}

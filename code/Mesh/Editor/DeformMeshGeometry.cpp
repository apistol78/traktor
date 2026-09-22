/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/DeformMeshGeometry.h"

#include "Core/Math/Half.h"
#include "Mesh/DeformMesh.h"
#include "Model/Model.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"

#include <algorithm>
#include <cstring>

namespace traktor::mesh
{
namespace
{

void writeHalf4(half_t* out, const Vector4& v)
{
	out[0] = floatToHalf(v.x());
	out[1] = floatToHalf(v.y());
	out[2] = floatToHalf(v.z());
	out[3] = floatToHalf(v.w());
}

void writeDeformVertex(const model::Model* m, const model::Vertex& v, DeformMesh::DeformVertex* out)
{
	m->getPosition(v.getPosition()).xyz1().storeUnaligned(out->Position);
	if (v.getNormal() != model::c_InvalidIndex)
		writeHalf4(out->Normal, m->getNormal(v.getNormal()).xyz0());
	if (v.getTangent() != model::c_InvalidIndex)
		writeHalf4(out->Tangent, m->getNormal(v.getTangent()).xyz0());
	if (v.getBinormal() != model::c_InvalidIndex)
		writeHalf4(out->Binormal, m->getNormal(v.getBinormal()).xyz0());
	if (v.getTexCoord(0) != model::c_InvalidIndex)
	{
		const Vector2& texCoord = m->getTexCoord(v.getTexCoord(0));
		out->TexCoord0[0] = texCoord.x;
		out->TexCoord0[1] = texCoord.y;
	}
	if (v.getTexCoord(1) != model::c_InvalidIndex)
	{
		const Vector2& texCoord = m->getTexCoord(v.getTexCoord(1));
		out->TexCoord1[0] = texCoord.x;
		out->TexCoord1[1] = texCoord.y;
	}
	if (v.getColor() != model::c_InvalidIndex)
		writeHalf4(out->Color, m->getColor(v.getColor()));
	else
		writeHalf4(out->Color, Vector4::one());
}

}

void buildDeformGeometry(
	const model::Model* model,
	const model::Model* rtModel,
	uint32_t rtSharedVertexCount,
	uint32_t totalVertexCount,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	DeformGeometry& outGeometry)
{
	outGeometry.parts.resize(0);
	outGeometry.indices.resize(0);

	const uint32_t modelVertexCount = model->getVertexCount();

	AlignedVector< uint8_t > deformed(totalVertexCount, 0);
	AlignedVector< uint8_t > inPart(totalVertexCount, 0);

	for (const auto& mt : materialTechniqueMap)
	{
		for (const auto& mtt : mt.second)
		{
			if (!mtt.deform)
				continue;

			MeshResource::DeformPart part;
			part.shaderTechnique = mtt.shaderTechnique;
			part.indexOffset = (uint32_t)outGeometry.indices.size();

			std::fill(inPart.begin(), inPart.end(), 0);
			const auto addVertex = [&](uint32_t vertex) {
				if (vertex >= totalVertexCount || inPart[vertex])
					return;
				inPart[vertex] = 1;
				deformed[vertex] = 1;
				outGeometry.indices.push_back(vertex);
			};

			for (const auto& polygon : model->getPolygons())
			{
				if (model->getMaterial(polygon.getMaterial()).getName() != mt.first)
					continue;
				for (uint32_t vertex : polygon.getVertices())
					addVertex(vertex);
			}

			// The ray tracing geometry references the appended vertices of the ray tracing
			// model (cut alpha, or a separately reduced model); those must deform as well or
			// the acceleration structure lags behind the rasterized mesh. Same vertex id
			// mapping as when building the ray tracing geometry.
			if (rtModel != nullptr)
			{
				const uint32_t rtMaterialId = rtModel->findMaterial(mt.first);
				if (rtMaterialId != model::c_InvalidIndex)
				{
					for (const auto& polygon : rtModel->getPolygonsByMaterial(rtMaterialId))
						for (uint32_t vertexId : polygon.getVertices())
							addVertex((vertexId < rtSharedVertexCount) ? vertexId : (modelVertexCount + (vertexId - rtSharedVertexCount)));
				}
			}

			part.indexCount = (uint32_t)outGeometry.indices.size() - part.indexOffset;
			if (part.indexCount > 0)
				outGeometry.parts.push_back(part);
		}
	}

	if (outGeometry.parts.empty())
	{
		outGeometry.indices.resize(0);
		return;
	}

	// Remaining vertices copy their undeformed positions.
	MeshResource::DeformPart part;
	part.indexOffset = (uint32_t)outGeometry.indices.size();
	for (uint32_t i = 0; i < totalVertexCount; ++i)
		if (!deformed[i])
			outGeometry.indices.push_back(i);
	part.indexCount = (uint32_t)outGeometry.indices.size() - part.indexOffset;
	if (part.indexCount > 0)
		outGeometry.parts.push_back(part);
}

uint32_t getDeformVerticesSize(uint32_t totalVertexCount)
{
	return totalVertexCount * sizeof(DeformMesh::DeformVertex);
}

uint32_t getDeformIndicesSize(const DeformGeometry& geometry)
{
	return (uint32_t)geometry.indices.size() * sizeof(uint32_t);
}

void writeDeformGeometry(
	render::Mesh* renderMesh,
	const model::Model* model,
	const model::Model* rtModel,
	uint32_t rtSharedVertexCount,
	const DeformGeometry& geometry)
{
	render::Buffer* verticesBuffer = renderMesh->getAuxBuffer(DeformMesh::c_fccDeformVertices);
	render::Buffer* indicesBuffer = renderMesh->getAuxBuffer(DeformMesh::c_fccDeformIndices);
	T_ASSERT(verticesBuffer != nullptr && indicesBuffer != nullptr);

	DeformMesh::DeformVertex* deformVertex = (DeformMesh::DeformVertex*)verticesBuffer->lock();
	std::memset(deformVertex, 0, verticesBuffer->getBufferSize());

	for (const auto& v : model->getVertices())
		writeDeformVertex(model, v, deformVertex++);

	// Appended ray tracing vertices; the ray tracing model carries their full attributes
	// (the vertex stream only holds their positions) which the deforming surface may read.
	if (rtModel != nullptr)
		for (uint32_t i = rtSharedVertexCount; i < rtModel->getVertexCount(); ++i)
			writeDeformVertex(rtModel, rtModel->getVertex(i), deformVertex++);

	verticesBuffer->unlock();

	uint32_t* deformIndex = (uint32_t*)indicesBuffer->lock();
	std::memcpy(deformIndex, geometry.indices.c_ptr(), geometry.indices.size() * sizeof(uint32_t));
	indicesBuffer->unlock();
}

}

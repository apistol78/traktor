/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"

#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/RayTracingMeshGeometry.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "World/WorldTypes.h"

#include <cstring>
#include <set>
#include <limits>

namespace traktor::mesh
{
namespace
{

const Guid c_shaderInstanceMeshDraw(L"{A8FDE33C-D75B-4D4E-848F-7D7CF97F11D0}");
const Guid c_shaderInstanceMeshCompact(L"{F46125D0-6321-4A7D-90BC-08C36ED9163C}");

}

Ref< MeshResource > InstanceMeshConverter::createResource() const
{
	return new InstanceMeshResource();
}

bool InstanceMeshConverter::getOperations(const MeshAsset* meshAsset, bool editor, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.push_back(new model::Triangulate());
	if (!editor)
		outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool InstanceMeshConverter::convert(
	const MeshAsset* meshAsset,
	const model::Model* model,
	const model::Model* rtModel,
	uint32_t rtSharedVertexCount,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	const AlignedVector< render::VertexElement >& depthVertexElements,
	MeshResource* meshResource,
	IStream* meshResourceStream) const
{
	// Create render mesh.
	const uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT(vertexSize > 0);

	const uint32_t modelVertexCount = (uint32_t)model->getVertices().size();
	const uint32_t polygonCount = (uint32_t)model->getPolygons().size();

	// Build ray tracing geometry; surfaces using alpha tested materials are cut and
	// tessellated to their coverage mask so ray tracing shaders don't need to perform
	// any alpha testing. The generated geometry is appended after the raster geometry
	// into the same vertex and index buffers.
	AlignedVector< resource::Id< render::ITexture > > albedoTextures;
	RayTracingGeometry rtGeometry;
	if (meshAsset->getEnableRaytracing())
		buildRayTracingGeometry(rtModel, materialTechniqueMap, modelVertexCount, rtSharedVertexCount, albedoTextures, rtGeometry);

	const uint32_t totalVertexCount = modelVertexCount + (uint32_t)rtGeometry.extraPositions.size();

	const bool useLargeIndices = (bool)(totalVertexCount >= 65536);
	const uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	// Depth-only techniques are rendered from a separate, tightly packed, stream which
	// carry only the attributes they read; the pipeline derive its declaration.
	const uint32_t depthVertexSize = render::getVertexSize(depthVertexElements);

	// Create render mesh.
	const uint32_t vertexBufferSize = totalVertexCount * vertexSize;
	const uint32_t depthVertexBufferSize = totalVertexCount * depthVertexSize;
	const uint32_t indexBufferSize = (polygonCount * 3 + (uint32_t)rtGeometry.indices.size()) * indexSize;
	const uint32_t rtVertexAttributesSize = (uint32_t)rtGeometry.materials.size() * sizeof(world::HWRT_Material);

	// Only declare the ray tracing vertex attribute buffer when there is geometry for it;
	// a zero-size buffer would fail to allocate at runtime.
	SmallMap< FourCC, uint32_t > auxBufferSizes;
	if (rtVertexAttributesSize > 0)
		auxBufferSizes[IMesh::c_fccRayTracingVertexAttributes] = rtVertexAttributesSize;

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		depthVertexElements,
		depthVertexBufferSize,
		useLargeIndices ? render::IndexType::UInt32 : render::IndexType::UInt16,
		indexBufferSize,
		auxBufferSizes);

	// Create vertex buffer.
	uint8_t* vertex = (uint8_t*)renderMesh->getVertexBuffer()->lock();
	std::memset(vertex, 0, vertexBufferSize);

	uint8_t* depthVertex = nullptr;
	if (depthVertexBufferSize > 0)
	{
		depthVertex = (uint8_t*)renderMesh->getDepthVertexBuffer()->lock();
		std::memset(depthVertex, 0, depthVertexBufferSize);
	}

	// Both streams are written through this; attributes absent from a declaration are
	// skipped, so the depth stream automatically get its subset and the two cannot
	// disagree on the attributes they share.
	const auto writeVertex = [&](const AlignedVector< render::VertexElement >& elements, uint8_t* vertex, const model::Vertex& v) {
		writeVertexData(elements, vertex, render::DataUsage::Position, 0, model->getPosition(v.getPosition()));
		if (v.getNormal() != model::c_InvalidIndex)
			writeVertexData(elements, vertex, render::DataUsage::Normal, 0, model->getNormal(v.getNormal()));
		if (v.getTangent() != model::c_InvalidIndex)
			writeVertexData(elements, vertex, render::DataUsage::Tangent, 0, model->getNormal(v.getTangent()));
		if (v.getBinormal() != model::c_InvalidIndex)
			writeVertexData(elements, vertex, render::DataUsage::Binormal, 0, model->getNormal(v.getBinormal()));
		if (v.getColor() != model::c_InvalidIndex)
			writeVertexData(elements, vertex, render::DataUsage::Color, 0, model->getColor(v.getColor()));
		if (v.getTexCoord(0) != model::c_InvalidIndex)
			writeVertexData(elements, vertex, render::DataUsage::Custom, 0, model->getTexCoord(v.getTexCoord(0)));
		if (v.getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(elements, vertex, render::DataUsage::Custom, 1, model->getTexCoord(v.getTexCoord(1)));
	};

	for (const auto& v : model->getVertices())
	{
		writeVertex(vertexElements, vertex, v);
		vertex += vertexSize;

		if (depthVertex != nullptr)
		{
			writeVertex(depthVertexElements, depthVertex, v);
			depthVertex += depthVertexSize;
		}
	}

	// Append ray tracing vertices; only positions are needed to build the acceleration structure.
	for (const auto& position : rtGeometry.extraPositions)
	{
		writeVertexData(vertexElements, vertex, render::DataUsage::Position, 0, position);
		vertex += vertexSize;

		if (depthVertex != nullptr)
		{
			writeVertexData(depthVertexElements, depthVertex, render::DataUsage::Position, 0, position);
			depthVertex += depthVertexSize;
		}
	}

	renderMesh->getVertexBuffer()->unlock();
	if (depthVertexBufferSize > 0)
		renderMesh->getDepthVertexBuffer()->unlock();

	// Create index buffer.
	std::map< std::wstring, AlignedVector< IndexRange > > techniqueRanges;

	uint8_t* index = (uint8_t*)renderMesh->getIndexBuffer()->lock();
	uint8_t* indexFirst = index;

	for (const auto& mt : materialTechniqueMap)
	{
		IndexRange range;
		range.offsetFirst = (uint32_t)(index - indexFirst) / indexSize;
		range.offsetLast = 0;

		for (const auto& polygon : model->getPolygons())
		{
			T_ASSERT(polygon.getVertices().size() == 3);

			if (model->getMaterial(polygon.getMaterial()).getName() != mt.first)
				continue;

			for (int32_t k = 0; k < 3; ++k)
			{
				if (useLargeIndices)
					*(uint32_t*)index = polygon.getVertex(k);
				else
					*(uint16_t*)index = polygon.getVertex(k);

				index += indexSize;
			}
		}

		range.offsetLast = (uint32_t)(index - indexFirst) / indexSize;
		if (range.offsetLast <= range.offsetFirst)
			continue;

		for (const auto& mtt : mt.second)
		{
			const std::wstring technique = mtt.worldTechnique + L"/" + mtt.shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	// Append ray tracing indices after the raster indices.
	const uint32_t rtIndexOffset = (uint32_t)(index - indexFirst) / indexSize;
	for (uint32_t i = 0; i < (uint32_t)rtGeometry.indices.size(); ++i)
	{
		if (useLargeIndices)
			*(uint32_t*)index = rtGeometry.indices[i];
		else
			*(uint16_t*)index = (uint16_t)rtGeometry.indices[i];

		index += indexSize;
	}

	renderMesh->getIndexBuffer()->unlock();

	// Build parts.
	AlignedVector< render::Primitives > meshPrimitives;
	SmallMap< std::wstring, InstanceMeshResource::parts_t > parts;

	// World techniques which the pipeline determined can be rendered from the depth stream.
	std::set< std::wstring > depthStreamTechniques;
	for (const auto& mt : materialTechniqueMap)
		for (const auto& mtt : mt.second)
			if (mtt.depthStream)
				depthStreamTechniques.insert(mtt.worldTechnique);

	for (const auto& techniqueRange : techniqueRanges)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(techniqueRange.first, L'/', worldTechnique, shaderTechnique);

		for (const auto& range : techniqueRange.second)
		{
			InstanceMeshResource::Part part;
			part.shaderTechnique = shaderTechnique;
			part.meshPart = (uint32_t)meshPrimitives.size();
			part.depthStream = (bool)(depthStreamTechniques.find(worldTechnique) != depthStreamTechniques.end());

			for (uint32_t k = 0; k < (uint32_t)meshPrimitives.size(); ++k)
			{
				if (
					meshPrimitives[k].offset == range.offsetFirst &&
					meshPrimitives[k].count == (range.offsetLast - range.offsetFirst) / 3)
				{
					part.meshPart = k;
					break;
				}
			}

			if (part.meshPart >= meshPrimitives.size())
			{
				meshPrimitives.push_back() = render::Primitives::setIndexed(
					render::PrimitiveType::Triangles,
					range.offsetFirst,
					(range.offsetLast - range.offsetFirst) / 3);
			}

			parts[worldTechnique].push_back(part);
		}
	}

	// Add ray tracing part; primitives reference the appended ray tracing index range,
	// and the vertex attributes match the acceleration structure primitive order. Left
	// empty when ray tracing is disabled for the asset so no acceleration structure is
	// built for the mesh at runtime.
	AlignedVector< render::RaytracingPrimitives > meshRaytracingPrimitives;
	if (!rtGeometry.indices.empty())
		meshRaytracingPrimitives.push_back() = { render::Primitives::setIndexed(
			render::PrimitiveType::Triangles,
			rtIndexOffset,
			(uint32_t)rtGeometry.indices.size() / 3), true };

	if (rtVertexAttributesSize > 0)
	{
		world::HWRT_Material* vptr = (world::HWRT_Material*)renderMesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->lock();
		std::memcpy(vptr, rtGeometry.materials.c_ptr(), rtGeometry.materials.size() * sizeof(world::HWRT_Material));
		renderMesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->unlock();
	}

	renderMesh->setPrimitives(meshPrimitives);
	renderMesh->setRaytracingPrimitives(meshRaytracingPrimitives);
	renderMesh->setBoundingBox(model->getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, renderMesh))
		return false;

	checked_type_cast< InstanceMeshResource* >(meshResource)->m_haveRenderMesh = true;
	checked_type_cast< InstanceMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< InstanceMeshResource* >(meshResource)->m_albedoTextures = albedoTextures;
	checked_type_cast< InstanceMeshResource* >(meshResource)->m_parts = parts;

	return true;
}

void InstanceMeshConverter::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_shaderInstanceMeshDraw, editor::PdfBuild);
	pipelineDepends->addDependency(c_shaderInstanceMeshCompact, editor::PdfBuild);
}

}

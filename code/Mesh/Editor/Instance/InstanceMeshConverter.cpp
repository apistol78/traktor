/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"
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

namespace traktor::mesh
{
	namespace
	{

const Guid c_shaderInstanceMeshDraw(L"{A8FDE33C-D75B-4D4E-848F-7D7CF97F11D0}");

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
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	MeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	// Create render mesh.
	const uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT(vertexSize > 0);

	const bool useLargeIndices = (bool)(model->getVertexCount() >= 65536);
	const uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	// Create render mesh.
	const uint32_t vertexBufferSize = (uint32_t)(model->getVertices().size() * vertexSize);
	const uint32_t indexBufferSize = (uint32_t)(model->getPolygons().size() * 3 * indexSize);
	const uint32_t rtTriangleAttributesSize = (uint32_t)(model->getPolygons().size() * sizeof(world::RTTriangleAttributes));
	const uint32_t rtVertexAttributesSize = (uint32_t)(model->getPolygons().size() * 3 * sizeof(world::RTVertexAttributes));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		useLargeIndices ? render::IndexType::UInt32 : render::IndexType::UInt16,
		indexBufferSize,
		{
			{ IMesh::c_fccRayTracingTriangleAttributes, rtTriangleAttributesSize },
			{ IMesh::c_fccRayTracingVertexAttributes, rtVertexAttributesSize }
		}
	);

	// Create vertex buffer.
	uint8_t* vertex = (uint8_t*)renderMesh->getVertexBuffer()->lock();
	std::memset(vertex, 0, vertexBufferSize);

	for (const auto& v : model->getVertices())
	{
		writeVertexData(vertexElements, vertex, render::DataUsage::Position, 0, model->getPosition(v.getPosition()));
		if (v.getNormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Normal, 0, model->getNormal(v.getNormal()));
		if (v.getTangent() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Tangent, 0, model->getNormal(v.getTangent()));
		if (v.getBinormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Binormal, 0, model->getNormal(v.getBinormal()));
		if (v.getColor() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Color, 0, model->getColor(v.getColor()));
		if (v.getTexCoord(0) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 0, model->getTexCoord(v.getTexCoord(0)));
		if (v.getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 1, model->getTexCoord(v.getTexCoord(1)));

		vertex += vertexSize;
	}

	renderMesh->getVertexBuffer()->unlock();

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

	renderMesh->getIndexBuffer()->unlock();

	// Build parts.
	AlignedVector< render::Mesh::Part > meshParts;
	std::map< std::wstring, InstanceMeshResource::parts_t > parts;

	for (const auto& techniqueRange : techniqueRanges)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(techniqueRange.first, L'/', worldTechnique, shaderTechnique);

		for (const auto& range : techniqueRange.second)
		{
			InstanceMeshResource::Part part;
			part.shaderTechnique = shaderTechnique;
			part.meshPart = (uint32_t)meshParts.size();

			for (uint32_t k = 0; k < (uint32_t)meshParts.size(); ++k)
			{
				if (
					meshParts[k].primitives.offset == range.offsetFirst &&
					meshParts[k].primitives.count == (range.offsetLast - range.offsetFirst) / 3
				)
				{
					part.meshPart = k;
					break;
				}
			}

			if (part.meshPart >= meshParts.size())
			{
				render::Mesh::Part meshPart;
				meshPart.name = techniqueRange.first;
				meshPart.primitives = render::Primitives::setIndexed(
					render::PrimitiveType::Triangles,
					range.offsetFirst,
					(range.offsetLast - range.offsetFirst) / 3
				);
				meshParts.push_back(meshPart);
			}

			parts[worldTechnique].push_back(part);
		}
	}

	// Add ray tracing part.
	{
		render::Mesh::Part meshPart;
		meshPart.name = L"__RT__";
		meshPart.primitives = render::Primitives::setIndexed(
			render::PrimitiveType::Triangles,
			0,
			model->getPolygons().size()
		);
		meshParts.push_back(meshPart);

		world::RTTriangleAttributes* tptr = (world::RTTriangleAttributes*)renderMesh->getAuxBuffer(IMesh::c_fccRayTracingTriangleAttributes)->lock();
		world::RTVertexAttributes* vptr = (world::RTVertexAttributes*)renderMesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->lock();

		for (const auto& mt : materialTechniqueMap)
		{
			for (uint32_t i = 0; i < model->getPolygons().size(); ++i)
			{
				const auto& polygon = model->getPolygon(i);
				const auto& material = model->getMaterial(polygon.getMaterial());

				if (material.getName() != mt.first)
					continue;

				Vector4 albedo = material.getColor();
				for (const auto& vertex : polygon.getVertices())
				{
					const uint32_t colorId = model->getVertex(vertex).getColor();
					if (colorId != model::c_InvalidIndex)
					{
						albedo = model->getColor(colorId);
						break;
					}
				}

				model->getNormal(polygon.getNormal()).storeUnaligned(tptr->normal);
				albedo.storeUnaligned(tptr->albedo);

				++tptr;

				for (uint32_t j = 0; j < 3; ++j)
				{
					const auto& vertex = model->getVertex(polygon.getVertex(j));

					model->getNormal(vertex.getNormal()).storeUnaligned(vptr->normal);

					if (vertex.getColor() != model::c_InvalidIndex)
						model->getColor(vertex.getColor()).storeUnaligned(vptr->albedo);
					else
						albedo.storeUnaligned(vptr->albedo);

					++vptr;
				}
			}
		}

		renderMesh->getAuxBuffer(IMesh::c_fccRayTracingTriangleAttributes)->unlock();
		renderMesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->unlock();
	}

	renderMesh->setParts(meshParts);
	renderMesh->setBoundingBox(model->getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, renderMesh))
		return false;

	checked_type_cast< InstanceMeshResource* >(meshResource)->m_haveRenderMesh = true;
	checked_type_cast< InstanceMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< InstanceMeshResource* >(meshResource)->m_parts = parts;

	return true;
}

void InstanceMeshConverter::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_shaderInstanceMeshDraw, editor::PdfBuild);
}

}

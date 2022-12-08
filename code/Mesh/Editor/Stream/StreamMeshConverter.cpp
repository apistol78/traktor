/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Model/Model.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Stream/StreamMeshConverter.h"
#include "Mesh/Stream/StreamMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"

namespace traktor
{
	namespace mesh
	{

Ref< MeshResource > StreamMeshConverter::createResource() const
{
	return new StreamMeshResource();
}

bool StreamMeshConverter::getOperations(const MeshAsset* meshAsset, bool editor, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.push_back(new model::Triangulate());
	if (!editor)
		outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::CalculateTangents(false));
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool StreamMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	int32_t maxInstanceCount,
	MeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	StreamMeshResource* streamMeshResource = checked_type_cast< StreamMeshResource*, false >(meshResource);

	AlignedVector< uint32_t > frameOffsets;
	Aabb3 boundingBox;

	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT(vertexSize > 0);

	for (uint32_t i = 0; i < models.size(); ++i)
	{
		model::Model model = *models[i];

		// Create render mesh.
		uint32_t vertexBufferSize = uint32_t(model.getVertices().size() * vertexSize);
		uint32_t indexBufferSize = uint32_t(model.getPolygons().size() * 3 * sizeof(uint16_t));

		Ref< render::Mesh > mesh = render::SystemMeshFactory().createMesh(
			vertexElements,
			vertexBufferSize,
			render::IndexType::UInt16,
			indexBufferSize
		);

		// Create vertex buffer.
		uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
		for (AlignedVector< model::Vertex >::const_iterator i = model.getVertices().begin(); i != model.getVertices().end(); ++i)
		{
			std::memset(vertex, 0, vertexSize);

			writeVertexData(vertexElements, vertex, render::DataUsage::Position, 0, model.getPosition(i->getPosition()));
			if (i->getNormal() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Normal, 0, model.getNormal(i->getNormal()));
			if (i->getTangent() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Tangent, 0, model.getNormal(i->getTangent()));
			if (i->getBinormal() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Binormal, 0, model.getNormal(i->getBinormal()));
			if (i->getColor() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Color, 0, model.getColor(i->getColor()));
			if (i->getTexCoord(0) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 0, model.getTexCoord(i->getTexCoord(0)));
			if (i->getTexCoord(1) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 1, model.getTexCoord(i->getTexCoord(1)));

			vertex += vertexSize;
		}
		mesh->getVertexBuffer()->unlock();

		// Create index buffer and build parts.
		AlignedVector< render::Mesh::Part > meshParts;

		uint16_t* index = static_cast< unsigned short* >(mesh->getIndexBuffer()->lock());
		uint16_t* indexFirst = index;

		for (AlignedVector< model::Material >::const_iterator i = model.getMaterials().begin(); i != model.getMaterials().end(); ++i)
		{
			const model::Material& material = *i;

			std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator materialIt = materialTechniqueMap.find(material.getName());
			if (materialIt == materialTechniqueMap.end())
				continue;

			int offset = int(index - indexFirst);
			int triangleCount = 0;

			int minIndex =  std::numeric_limits< int >::max();
			int maxIndex = -std::numeric_limits< int >::max();

			for (AlignedVector< model::Polygon >::const_iterator j = model.getPolygons().begin(); j != model.getPolygons().end(); ++j)
			{
				const model::Polygon& polygon = *j;
				T_ASSERT(polygon.getVertices().size() == 3);

				if (polygon.getMaterial() != std::distance< AlignedVector< model::Material >::const_iterator >(model.getMaterials().begin(), i))
					continue;

				for (int k = 0; k < 3; ++k)
				{
					*index++ = uint16_t(polygon.getVertex(k));
					minIndex = std::min< int >(minIndex, polygon.getVertex(k));
					maxIndex = std::max< int >(maxIndex, polygon.getVertex(k));
				}

				triangleCount++;
			}

			if (!triangleCount)
				continue;

			meshParts.push_back(render::Mesh::Part());
			meshParts.back().name = material.getName();
			meshParts.back().primitives.setIndexed(
				render::PrimitiveType::Triangles,
				offset,
				triangleCount,
				minIndex,
				maxIndex
			);
		}

		mesh->setParts(meshParts);
		mesh->getIndexBuffer()->unlock();
		mesh->setBoundingBox(model.getBoundingBox());

		int32_t frameOffset = meshResourceStream->tell();

		if (!render::MeshWriter().write(meshResourceStream, mesh))
			return false;

		frameOffsets.push_back(frameOffset);
		boundingBox.contain(mesh->getBoundingBox());
	}

	streamMeshResource->m_shader = resource::Id< render::Shader >(materialGuid);
	streamMeshResource->m_frameOffsets = frameOffsets;
	streamMeshResource->m_boundingBox = boundingBox;

	// Create resource parts.
	for (std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator i = materialTechniqueMap.begin(); i != materialTechniqueMap.end(); ++i)
	{
		for (std::list< MeshMaterialTechnique >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			StreamMeshResource::Part part;
			part.shaderTechnique = j->shaderTechnique;
			part.meshPart = i->first;
			streamMeshResource->m_parts[j->worldTechnique].push_back(part);
		}
	}

	return true;
}

	}
}

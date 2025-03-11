/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/Skinned/SkinnedMeshConverter.h"

#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
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

#include <algorithm>
#include <cstring>
#include <limits>

namespace traktor::mesh
{
namespace
{

const resource::Id< render::Shader > c_shaderUpdateSkin(L"{E520B46A-24BC-764C-A3E2-819DB57B7515}");

}

Ref< MeshResource > SkinnedMeshConverter::createResource() const
{
	return new SkinnedMeshResource();
}

bool SkinnedMeshConverter::getOperations(const MeshAsset* meshAsset, bool editor, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.push_back(new model::Triangulate());
	if (!editor)
		outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool SkinnedMeshConverter::convert(
	const MeshAsset* meshAsset,
	const model::Model* model,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	MeshResource* meshResource,
	IStream* meshResourceStream) const
{
	// Create render mesh.
	const uint32_t vertexSize = render::getVertexSize(vertexElements);

	const bool useLargeIndices = (bool)(model->getVertexCount() >= 65536);
	const uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	const uint32_t vertexBufferSize = (uint32_t)(model->getVertices().size() * vertexSize);
	const uint32_t indexBufferSize = (uint32_t)(model->getPolygons().size() * 3 * indexSize);
	const uint32_t auxBufferSize = (uint32_t)(model->getVertices().size() * (6 * 4 * sizeof(float)));
	const uint32_t rtVertexAttributesSize = (uint32_t)(model->getPolygons().size() * 3 * sizeof(world::RTVertexAttributes));

	Ref< render::Mesh > mesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		useLargeIndices ? render::IndexType::UInt32 : render::IndexType::UInt16,
		indexBufferSize,
		{ { SkinnedMesh::c_fccSkinPosition, auxBufferSize },
			{ IMesh::c_fccRayTracingVertexAttributes, rtVertexAttributesSize } });

	// Create vertex and aux buffers.
	uint8_t* vertex = nullptr;
	if (vertexBufferSize > 0)
	{
		vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
		std::memset(vertex, 0, vertexBufferSize);
	}

	float* aux = static_cast< float* >(mesh->getAuxBuffer(SkinnedMesh::c_fccSkinPosition)->lock());
	std::memset(aux, 0, auxBufferSize);

	AlignedVector< std::pair< uint32_t, float > > jointInfluences;
	for (const auto& v : model->getVertices())
	{
		// Write vertex data.
		if (vertex != nullptr)
		{
			if (v.getColor() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Color, 0, model->getColor(v.getColor()));
			if (v.getTexCoord(0) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 2, model->getTexCoord(v.getTexCoord(0)));
			if (v.getTexCoord(1) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 3, model->getTexCoord(v.getTexCoord(1)));
			vertex += vertexSize;
		}

		// Gather joints and write weights to vertex.
		uint32_t jointCount = model->getJointCount();

		jointInfluences.resize(0);
		for (uint32_t i = 0; i < jointCount; ++i)
		{
			const float w = v.getJointInfluence(i);
			if (std::abs(w) > FUZZY_EPSILON)
				jointInfluences.push_back(std::make_pair(i, w));
		}

		std::sort(jointInfluences.begin(), jointInfluences.end(), [](const std::pair< int, float >& lh, const std::pair< int, float >& rh) {
			return lh.second > rh.second;
		});

		jointCount = (uint32_t)jointInfluences.size();
		jointCount = std::min< uint32_t >(4, jointCount);

		float totalInfluence = 0.0f;
		for (uint32_t j = 0; j < jointCount; ++j)
			totalInfluence += jointInfluences[j].second;

		float blendIndices[4], blendWeights[4];
		if (std::abs(totalInfluence) > FUZZY_EPSILON)
		{
			// Don't normalize single bone vertices; skinned with world.
			if (jointCount <= 1)
				totalInfluence = 1.0f;

			for (uint32_t i = 0; i < jointCount; ++i)
			{
				blendIndices[i] = (float)jointInfluences[i].first;
				blendWeights[i] = jointInfluences[i].second / totalInfluence;
			}

			for (uint32_t i = jointCount; i < 4; ++i)
				blendIndices[i] =
					blendWeights[i] = 0.0f;
		}
		else
		{
			for (uint32_t i = jointCount; i < 4; ++i)
				blendIndices[i] =
					blendWeights[i] = 0.0f;
		}

		// Write aux data.
		{
			const Vector4 position = model->getPosition(v.getPosition());
			const Vector4 normal = (v.getNormal() != model::c_InvalidIndex) ? model->getNormal(v.getNormal()) : Vector4::zero();
			const Vector4 tangent = (v.getTangent() != model::c_InvalidIndex) ? model->getNormal(v.getTangent()) : Vector4::zero();
			const Vector4 binormal = (v.getBinormal() != model::c_InvalidIndex) ? model->getNormal(v.getBinormal()) : Vector4::zero();
			position.storeUnaligned(aux);
			aux += 4;
			normal.storeUnaligned(aux);
			aux += 4;
			tangent.storeUnaligned(aux);
			aux += 4;
			binormal.storeUnaligned(aux);
			aux += 4;
			std::memcpy(aux, blendIndices, 4 * sizeof(float));
			aux += 4;
			std::memcpy(aux, blendWeights, 4 * sizeof(float));
			aux += 4;
		}
	}

	if (vertex)
		mesh->getVertexBuffer()->unlock();

	// Create index buffer.
	std::map< std::wstring, AlignedVector< IndexRange > > techniqueRanges;

	uint8_t* index = (uint8_t*)mesh->getIndexBuffer()->lock();
	uint8_t* indexFirst = index;

	for (const auto& mt : materialTechniqueMap)
	{
		IndexRange range;
		range.offsetFirst = uint32_t(index - indexFirst) / indexSize;
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

		range.offsetLast = uint32_t(index - indexFirst) / indexSize;
		if (range.offsetLast <= range.offsetFirst)
			continue;

		for (const auto& mtt : mt.second)
		{
			const std::wstring technique = mtt.worldTechnique + L"/" + mtt.shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	mesh->getIndexBuffer()->unlock();

	// Build parts.
	AlignedVector< render::Mesh::Part > meshParts;
	SmallMap< std::wstring, SkinnedMeshResource::parts_t > parts;

	for (std::map< std::wstring, AlignedVector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (AlignedVector< IndexRange >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			SkinnedMeshResource::Part part;
			part.shaderTechnique = shaderTechnique;
			part.meshPart = uint32_t(meshParts.size());

			for (uint32_t k = 0; k < uint32_t(meshParts.size()); ++k)
			{
				if (
					meshParts[k].primitives.offset == j->offsetFirst &&
					meshParts[k].primitives.count == (j->offsetLast - j->offsetFirst) / 3)
				{
					part.meshPart = k;
					break;
				}
			}

			if (part.meshPart >= meshParts.size())
			{
				render::Mesh::Part meshPart;
				meshPart.primitives = render::Primitives::setIndexed(
					render::PrimitiveType::Triangles,
					j->offsetFirst,
					(j->offsetLast - j->offsetFirst) / 3);
				meshParts.push_back(meshPart);
			}

			parts[worldTechnique].push_back(part);
		}
	}

	// Add ray tracing part.
	AlignedVector< resource::Id< render::ITexture > > albedoTextures;
	{
		render::Mesh::Part meshPart;
		meshPart.name = L"__RT__";
		meshPart.primitives = render::Primitives::setIndexed(
			render::PrimitiveType::Triangles,
			0,
			model->getPolygons().size());
		meshParts.push_back(meshPart);

		world::RTVertexAttributes* vptr = (world::RTVertexAttributes*)mesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->lock();

		for (const auto& mt : materialTechniqueMap)
		{
			const uint32_t materialId = model->findMaterial(mt.first);
			if (materialId == model::c_InvalidIndex)
				continue;

			const auto& material = model->getMaterial(materialId);

			// Look up index of albedo map, if map doesn't exist add a new reference.
			int32_t albedoMapId = -1;
			if (material.getDiffuseMap().texture.isNotNull())
			{
				const auto it = std::find(albedoTextures.begin(), albedoTextures.end(), resource::Id< render::ITexture >(material.getDiffuseMap().texture));
				if (it != albedoTextures.end())
					albedoMapId = (int32_t)std::distance(albedoTextures.begin(), it);
				else
				{
					albedoMapId = (int32_t)albedoTextures.size();
					albedoTextures.push_back(resource::Id< render::ITexture >(material.getDiffuseMap().texture));
				}
			}

			for (const auto& polygon : model->getPolygonsByMaterial(materialId))
			{
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

				for (uint32_t j = 0; j < 3; ++j)
				{
					const auto& vertex = model->getVertex(polygon.getVertex(j));

					model->getNormal(vertex.getNormal()).storeUnaligned(vptr->normal);

					if (vertex.getColor() != model::c_InvalidIndex)
						model->getColor(vertex.getColor()).storeUnaligned(vptr->albedo);
					else
						albedo.storeUnaligned(vptr->albedo);

					vptr->albedo[3] = material.getEmissive();

					vptr->texCoord[0] = vptr->texCoord[1] = 0.0f;
					if (vertex.getTexCoord(0) != model::c_InvalidIndex)
					{
						const Vector2 texCoord = model->getTexCoord(vertex.getTexCoord(0));
						vptr->texCoord[0] = texCoord.x;
						vptr->texCoord[1] = texCoord.y;
					}

					vptr->albedoMap = albedoMapId;

					++vptr;
				}
			}
		}

		mesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->unlock();
	}

	mesh->setParts(meshParts);
	mesh->setBoundingBox(model->getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_albedoTextures = albedoTextures;
	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_parts = parts;
	for (uint32_t i = 0; i < model->getJointCount(); ++i)
		checked_type_cast< SkinnedMeshResource* >(meshResource)->m_jointMap[model->getJoint(i).getName()] = i;

	return true;
}

void SkinnedMeshConverter::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_shaderUpdateSkin, editor::PdfBuild);
}

}

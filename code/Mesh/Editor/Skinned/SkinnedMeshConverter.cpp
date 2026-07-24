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
#include "Mesh/Editor/RayTracingMeshGeometry.h"
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

/*! Write a single skinning vertex (6 x float4: position, normal, tangent, binormal, blend indices, blend weights).
 *
 * \a influences is the (joint, weight) list for the vertex; it is sorted in place, reduced to the four
 * strongest joints and normalized before being written.
 */
void writeSkinVertex(
	float*& aux,
	const Vector4& position,
	const Vector4& normal,
	const Vector4& tangent,
	const Vector4& binormal,
	AlignedVector< std::pair< uint32_t, float > >& influences)
{
	std::sort(influences.begin(), influences.end(), [](const std::pair< uint32_t, float >& lh, const std::pair< uint32_t, float >& rh) {
		return lh.second > rh.second;
	});

	const uint32_t jointCount = std::min< uint32_t >(4, (uint32_t)influences.size());

	float totalInfluence = 0.0f;
	for (uint32_t i = 0; i < jointCount; ++i)
		totalInfluence += influences[i].second;

	float blendIndices[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float blendWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (std::abs(totalInfluence) > FUZZY_EPSILON)
	{
		// Don't normalize single bone vertices; skinned with world.
		if (jointCount <= 1)
			totalInfluence = 1.0f;

		for (uint32_t i = 0; i < jointCount; ++i)
		{
			blendIndices[i] = (float)influences[i].first;
			blendWeights[i] = influences[i].second / totalInfluence;
		}
	}

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

	const uint32_t modelVertexCount = (uint32_t)model->getVertices().size();
	const uint32_t polygonCount = (uint32_t)model->getPolygons().size();

	// Build ray tracing geometry; surfaces using alpha tested materials are cut and
	// tessellated to their coverage mask so ray tracing shaders don't need to perform
	// any alpha testing. Generated vertices are appended after the model's own vertices
	// in the skinning buffer so they get deformed along with the rest of the mesh.
	AlignedVector< resource::Id< render::ITexture > > albedoTextures;
	RayTracingGeometry rtGeometry;
	buildRayTracingGeometry(model, materialTechniqueMap, modelVertexCount, albedoTextures, rtGeometry);

	const uint32_t rtVertexCount = (uint32_t)rtGeometry.extraVertexSources.size();
	const uint32_t totalSkinVertexCount = modelVertexCount + rtVertexCount;

	const bool useLargeIndices = (bool)(totalSkinVertexCount >= 65536);
	const uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	// The raster vertex buffer only holds the model's own vertices; ray tracing reads
	// deformed positions from the (larger) skinning buffer instead.
	const uint32_t vertexBufferSize = modelVertexCount * vertexSize;
	const uint32_t indexBufferSize = (polygonCount * 3 + (uint32_t)rtGeometry.indices.size()) * indexSize;
	const uint32_t auxBufferSize = (uint32_t)(totalSkinVertexCount * (6 * 4 * sizeof(float)));
	const uint32_t rtVertexAttributesSize = (uint32_t)rtGeometry.materials.size() * sizeof(world::HWRT_Material);

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
				writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 0, model->getTexCoord(v.getTexCoord(0)));
			if (v.getTexCoord(1) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 1, model->getTexCoord(v.getTexCoord(1)));
			vertex += vertexSize;
		}

		// Gather joint influences and write the skinning vertex.
		const uint32_t jointCount = model->getJointCount();

		jointInfluences.resize(0);
		for (uint32_t i = 0; i < jointCount; ++i)
		{
			const float w = v.getJointInfluence(i);
			if (std::abs(w) > FUZZY_EPSILON)
				jointInfluences.push_back(std::make_pair(i, w));
		}

		const Vector4 position = model->getPosition(v.getPosition());
		const Vector4 normal = (v.getNormal() != model::c_InvalidIndex) ? model->getNormal(v.getNormal()) : Vector4::zero();
		const Vector4 tangent = (v.getTangent() != model::c_InvalidIndex) ? model->getNormal(v.getTangent()) : Vector4::zero();
		const Vector4 binormal = (v.getBinormal() != model::c_InvalidIndex) ? model->getNormal(v.getBinormal()) : Vector4::zero();

		writeSkinVertex(aux, position, normal, tangent, binormal, jointInfluences);
	}

	// Append skinning vertices generated by the ray tracing coverage clipping; their
	// attributes are the barycentric blend of the originating triangle's vertices so
	// they deform together with the surface.
	{
		const uint32_t jointCount = model->getJointCount();

		AlignedVector< float > jointAccum;
		jointAccum.resize(jointCount);

		for (const auto& source : rtGeometry.extraVertexSources)
		{
			Vector4 position = Vector4::zero();
			Vector4 normal = Vector4::zero();
			Vector4 tangent = Vector4::zero();
			Vector4 binormal = Vector4::zero();

			for (uint32_t i = 0; i < jointCount; ++i)
				jointAccum[i] = 0.0f;

			for (int32_t k = 0; k < 3; ++k)
			{
				const Scalar w(source.weights[k]);
				const auto& sv = model->getVertex(source.vertices[k]);

				position += model->getPosition(sv.getPosition()) * w;
				if (sv.getNormal() != model::c_InvalidIndex)
					normal += model->getNormal(sv.getNormal()) * w;
				if (sv.getTangent() != model::c_InvalidIndex)
					tangent += model->getNormal(sv.getTangent()) * w;
				if (sv.getBinormal() != model::c_InvalidIndex)
					binormal += model->getNormal(sv.getBinormal()) * w;

				for (uint32_t i = 0; i < jointCount; ++i)
					jointAccum[i] += source.weights[k] * sv.getJointInfluence(i);
			}

			jointInfluences.resize(0);
			for (uint32_t i = 0; i < jointCount; ++i)
			{
				if (std::abs(jointAccum[i]) > FUZZY_EPSILON)
					jointInfluences.push_back(std::make_pair(i, jointAccum[i]));
			}

			writeSkinVertex(aux, position, normal, tangent, binormal, jointInfluences);
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

	mesh->getIndexBuffer()->unlock();

	// Build parts.
	AlignedVector< render::Primitives > meshPrimitives;
	SmallMap< std::wstring, SkinnedMeshResource::parts_t > parts;

	for (std::map< std::wstring, AlignedVector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (AlignedVector< IndexRange >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			SkinnedMeshResource::Part part;
			part.shaderTechnique = shaderTechnique;
			part.meshPart = uint32_t(meshPrimitives.size());

			for (uint32_t k = 0; k < uint32_t(meshPrimitives.size()); ++k)
			{
				if (
					meshPrimitives[k].offset == j->offsetFirst &&
					meshPrimitives[k].count == (j->offsetLast - j->offsetFirst) / 3)
				{
					part.meshPart = k;
					break;
				}
			}

			if (part.meshPart >= meshPrimitives.size())
			{
				meshPrimitives.push_back() = render::Primitives::setIndexed(
					render::PrimitiveType::Triangles,
					j->offsetFirst,
					(j->offsetLast - j->offsetFirst) / 3);
			}

			parts[worldTechnique].push_back(part);
		}
	}

	// Add ray tracing part; primitives reference the appended ray tracing index range,
	// and the vertex attributes match the acceleration structure primitive order.
	AlignedVector< render::RaytracingPrimitives > meshRaytracingPrimitives;
	meshRaytracingPrimitives.push_back() = { render::Primitives::setIndexed(
		render::PrimitiveType::Triangles,
		rtIndexOffset,
		(uint32_t)rtGeometry.indices.size() / 3), true };

	if (rtVertexAttributesSize > 0)
	{
		world::HWRT_Material* vptr = (world::HWRT_Material*)mesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->lock();
		std::memcpy(vptr, rtGeometry.materials.c_ptr(), rtGeometry.materials.size() * sizeof(world::HWRT_Material));
		mesh->getAuxBuffer(IMesh::c_fccRayTracingVertexAttributes)->unlock();
	}

	mesh->setPrimitives(meshPrimitives);
	mesh->setRaytracingPrimitives(meshRaytracingPrimitives);
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

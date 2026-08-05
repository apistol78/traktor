/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/RayTracingMeshGeometry.h"

#include <algorithm>

#include "Core/Math/Vector2.h"
#include "Model/Material.h"
#include "Model/Model.h"

namespace traktor::mesh
{
namespace
{

/*! Build a HWRT_Material from a vertex's shading attributes. */
world::HWRT_Material makeMaterial(const Vector4& normal, const Vector4& albedo, const Vector2& texCoord, float emissive, int32_t albedoMapId)
{
	world::HWRT_Material m = {};
	normal.normalized().storeUnaligned3(m.normal);
	albedo.storeUnaligned3(m.albedo);
	m.emissive = emissive;
	m.albedoMap = albedoMapId;
	m.texCoord[0] = texCoord.x;
	m.texCoord[1] = texCoord.y;
	return m;
}

}

void buildRayTracingGeometry(
	const model::Model* model,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	uint32_t rasterVertexCount,
	uint32_t sharedVertexCount,
	AlignedVector< resource::Id< render::ITexture > >& outAlbedoTextures,
	RayTracingGeometry& outGeometry)
{
	// Vertices beyond the shared prefix are emitted as extra positions, appended after the raster
	// geometry in the buffer, so model vertex i maps to buffer slot:
	//   i < sharedVertexCount            -> i                                  (a shared raster vertex)
	//   i >= sharedVertexCount           -> rasterVertexCount + (i - shared)   (an appended extra)
	const uint32_t vertexCount = model->getVertexCount();
	outGeometry.extraPositions.reserve(vertexCount - sharedVertexCount);
	for (uint32_t i = sharedVertexCount; i < vertexCount; ++i)
		outGeometry.extraPositions.push_back(model->getVertexPosition(i));

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
			const resource::Id< render::ITexture > diffuseId(material.getDiffuseMap().texture);
			const auto it = std::find(outAlbedoTextures.begin(), outAlbedoTextures.end(), diffuseId);
			if (it != outAlbedoTextures.end())
				albedoMapId = (int32_t)std::distance(outAlbedoTextures.begin(), it);
			else
			{
				albedoMapId = (int32_t)outAlbedoTextures.size();
				outAlbedoTextures.push_back(diffuseId);
			}
		}

		const float emissive = material.getEmissive();
		const Vector4 materialColor = material.getColor();

		// The model is already cut, so every triangle is emitted directly - indices reference the
		// model's own vertices (shared prefix in place, the rest as extra positions).
		for (const auto& polygon : model->getPolygonsByMaterial(materialId))
		{
			if (polygon.getVertices().size() != 3)
				continue;

			for (uint32_t j = 0; j < 3; ++j)
			{
				const uint32_t vertexId = polygon.getVertex(j);
				const auto& vertex = model->getVertex(vertexId);

				const Vector4 normal = (vertex.getNormal() != model::c_InvalidIndex) ? model->getNormal(vertex.getNormal()) : Vector4(0.0f, 1.0f, 0.0f, 0.0f);
				const Vector4 albedo = (vertex.getColor() != model::c_InvalidIndex) ? model->getColor(vertex.getColor()) : materialColor;
				const Vector2 texCoord = (vertex.getTexCoord(0) != model::c_InvalidIndex) ? model->getTexCoord(vertex.getTexCoord(0)) : Vector2(0.0f, 0.0f);

				const uint32_t bufferIndex = (vertexId < sharedVertexCount) ? vertexId : (rasterVertexCount + (vertexId - sharedVertexCount));
				outGeometry.indices.push_back(bufferIndex);
				outGeometry.materials.push_back(makeMaterial(normal, albedo, texCoord, emissive, albedoMapId));
			}
		}
	}
}

}

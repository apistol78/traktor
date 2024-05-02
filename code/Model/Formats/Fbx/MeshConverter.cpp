/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Model/Model.h"
#include "Model/Formats/Fbx/Conversion.h"
#include "Model/Formats/Fbx/MaterialConverter.h"
#include "Model/Formats/Fbx/MeshConverter.h"

namespace traktor::model
{
	namespace
	{

Matrix44 normalize(const Matrix44& m)
{
	return Matrix44(
		m.axisX().normalized(),
		m.axisY().normalized(),
		m.axisZ().normalized(),
		m.translation()
	);
}

	}

bool convertMesh(
	Model& outModel,
	ufbx_scene* scene,
	ufbx_node* meshNode,
	const Matrix44& axisTransform
)
{
	T_FATAL_ASSERT(meshNode->mesh != nullptr);

	// Convert materials.
	SmallMap< int32_t, int32_t > materialMap;
	if (!convertMaterials(outModel, materialMap, meshNode))
		return false;

	const uint32_t positionBase = uint32_t(outModel.getPositions().size());

	const Matrix44 Mnode = convertMatrix(meshNode->geometry_to_world);
	const Matrix44 Mglobal = axisTransform * Mnode;
	const Matrix44 MglobalN = normalize(Mglobal);

	outModel.reservePositions(positionBase + meshNode->mesh->num_vertices);
	for (size_t i = 0; i <  meshNode->mesh->num_vertices; ++i)
	{
		const Vector4 v = Mglobal * convertPosition(meshNode->mesh->vertices[i]);
		outModel.addPosition(v);
	}

	// Convert joint vertex weights; model must contain joints.
	typedef SmallMap< uint32_t, float > bone_influences_t;

	AlignedVector< bone_influences_t > vertexJoints;
	vertexJoints.resize(meshNode->mesh->num_vertices);

	for (size_t i = 0; i < meshNode->mesh->skin_deformers.count; ++i)
	{
		ufbx_skin_deformer* skinDeformer = meshNode->mesh->skin_deformers.data[i];
		if (!skinDeformer)
			continue;

		for (size_t j = 0; j < skinDeformer->clusters.count; ++j)
		{
			ufbx_skin_cluster* cluster = skinDeformer->clusters.data[j];
			if (!cluster)
				continue;

			std::wstring jointName = mbstows(cluster->bone_node->name.data);
			const size_t p = jointName.find(L':');
			if (p != std::wstring::npos)
				jointName = jointName.substr(p + 1);

			const uint32_t jointIndex = outModel.findJointIndex(jointName);
			if (jointIndex == c_InvalidIndex)
			{
				log::warning << L"Unable to set vertex weight; no such joint \"" << jointName << L"\"." << Endl;
				continue;
			}

			for (size_t k = 0; k < cluster->num_weights; ++k)
			{
				const uint32_t vertexIndex = cluster->vertices.data[k];
				const float jointWeight = cluster->weights.data[k];
				if (jointWeight > FUZZY_EPSILON)
					vertexJoints[vertexIndex].insert(std::pair< uint32_t, float >(jointIndex, jointWeight));
			}
		}
	}

	// Setup all texcoord channels.
	StaticVector< uint32_t, 64 > texCoordChannels;
	texCoordChannels.resize(meshNode->mesh->uv_sets.count);
	for (size_t k = 0; k < meshNode->mesh->uv_sets.count; ++k)
	{
		const ufbx_uv_set& uvSet = meshNode->mesh->uv_sets.data[k];
		if (uvSet.vertex_uv.exists)
		{
			const std::wstring channelName = mbstows(uvSet.name.data);
			const uint32_t channel = outModel.addUniqueTexCoordChannel(channelName);
			texCoordChannels[k] = channel;
		}
	}

	// Convert polygons.
	for (size_t i = 0; i < meshNode->mesh->num_faces; ++i)
	{
		const ufbx_face& face = meshNode->mesh->faces[i];

		if (face.num_indices > Polygon::vertices_t::Capacity)
		{
			log::warning << L"Too many vertices (" << face.num_indices << L", max " << Polygon::vertices_t::Capacity << L") in polygon, skipped." << Endl;
			continue;
		}

		Polygon polygon;

		if (meshNode->mesh->face_material.count > 0)
			polygon.setMaterial(materialMap[meshNode->mesh->face_material[i]]);

		for (uint32_t j = 0; j < face.num_indices; ++j)
		{
			const uint32_t vertexIndex = face.index_begin + j;
			const uint32_t pointIndex = meshNode->mesh->vertex_indices[vertexIndex];

			Vertex vertex;
			vertex.setPosition(positionBase + pointIndex);

			if (pointIndex < int32_t(vertexJoints.size()))
			{
				for (const auto& k : vertexJoints[pointIndex])
					vertex.setJointInfluence(k.first, k.second);
			}

			if (meshNode->mesh->vertex_color.exists)
			{
				vertex.setNormal(outModel.addUniqueColor(convertColor(
					ufbx_get_vertex_vec4(&meshNode->mesh->vertex_color, vertexIndex)
				)));
			}

			for (size_t k = 0; k < meshNode->mesh->uv_sets.count; ++k)
			{
				const ufbx_uv_set& uvSet = meshNode->mesh->uv_sets.data[k];
				if (uvSet.vertex_uv.exists)
				{
					const uint32_t channel = texCoordChannels[k];
					vertex.setTexCoord(channel, outModel.addUniqueTexCoord(convertVector2(
						ufbx_get_vertex_vec2(&uvSet.vertex_uv, vertexIndex)
					) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f)));
				}
			}

			if (meshNode->mesh->vertex_normal.exists)
			{
				vertex.setNormal(outModel.addUniqueNormal(MglobalN * convertNormal(
					ufbx_get_vertex_vec3(&meshNode->mesh->vertex_normal, vertexIndex)
				)));
			}

			if (meshNode->mesh->vertex_tangent.exists)
			{
				vertex.setTangent(outModel.addUniqueNormal(MglobalN * convertNormal(
					ufbx_get_vertex_vec3(&meshNode->mesh->vertex_tangent, vertexIndex)
				)));
			}

			if (meshNode->mesh->vertex_bitangent.exists)
			{
				vertex.setBinormal(outModel.addUniqueNormal(MglobalN * convertNormal(
					ufbx_get_vertex_vec3(&meshNode->mesh->vertex_bitangent, vertexIndex)
				)));
			}

			polygon.addVertex(outModel.addVertex(vertex));
		}

		outModel.addPolygon(polygon);
	}

	return true;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Gltf/GltfMeshConverter.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Model.h"

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
		m.translation());
}

bool unpackAccessorData(const cgltf_accessor* accessor, AlignedVector< cgltf_float >& outData)
{
	if (!accessor)
		return false;

	const cgltf_size componentCount = cgltf_num_components(accessor->type);
	const cgltf_size totalFloats = accessor->count * componentCount;

	outData.resize(totalFloats);
	const cgltf_size unpacked = cgltf_accessor_unpack_floats(accessor, outData.ptr(), totalFloats);

	return unpacked == totalFloats;
}

bool unpackIndices(const cgltf_accessor* accessor, AlignedVector< uint32_t >& outIndices)
{
	if (!accessor)
		return false;

	outIndices.resize(accessor->count);
	const cgltf_size unpacked = cgltf_accessor_unpack_indices(accessor, outIndices.ptr(), sizeof(uint32_t), accessor->count);

	return unpacked == accessor->count;
}

}

bool convertMesh(
	Model& outModel,
	const cgltf_data* data,
	const cgltf_node* node,
	const SmallMap< cgltf_size, int32_t >& materialMap,
	const Matrix44& axisTransform)
{
	if (!node->mesh)
		return false;

	const cgltf_mesh* mesh = node->mesh;
	const uint32_t positionBase = uint32_t(outModel.getPositions().size());

	// Get node transform
	const Matrix44 nodeTransform = getNodeTransform(node);
	const Matrix44 globalTransform = axisTransform * nodeTransform;
	const Matrix44 globalNormalTransform = normalize(globalTransform);

	// Setup texture coordinate channels
	StaticVector< uint32_t, 8 > texCoordChannels;
	for (uint32_t i = 0; i < 8; ++i)
	{
		std::wstring channelName = L"UV" + toString(i);
		texCoordChannels.push_back(outModel.addUniqueTexCoordChannel(channelName));
	}

	// Process each primitive in the mesh
	for (cgltf_size primIndex = 0; primIndex < mesh->primitives_count; ++primIndex)
	{
		const cgltf_primitive* primitive = &mesh->primitives[primIndex];

		// Only support triangles for now
		if (primitive->type != cgltf_primitive_type_triangles)
		{
			log::warning << L"Primitive type " << (int)primitive->type << L" not supported, skipping." << Endl;
			continue;
		}

		// Find required attributes
		const cgltf_accessor* positionAccessor = nullptr;
		const cgltf_accessor* normalAccessor = nullptr;
		const cgltf_accessor* tangentAccessor = nullptr;
		const cgltf_accessor* texCoordAccessors[8] = { nullptr };
		const cgltf_accessor* colorAccessor = nullptr;
		const cgltf_accessor* jointsAccessor = nullptr;
		const cgltf_accessor* weightsAccessor = nullptr;

		for (cgltf_size attrIndex = 0; attrIndex < primitive->attributes_count; ++attrIndex)
		{
			const cgltf_attribute* attr = &primitive->attributes[attrIndex];

			switch (attr->type)
			{
			case cgltf_attribute_type_position:
				positionAccessor = attr->data;
				break;
			case cgltf_attribute_type_normal:
				normalAccessor = attr->data;
				break;
			case cgltf_attribute_type_tangent:
				tangentAccessor = attr->data;
				break;
			case cgltf_attribute_type_texcoord:
				if (attr->index >= 0 && attr->index < 8)
					texCoordAccessors[attr->index] = attr->data;
				break;
			case cgltf_attribute_type_color:
				if (attr->index == 0) // Only support COLOR_0 for now
					colorAccessor = attr->data;
				break;
			case cgltf_attribute_type_joints:
				if (attr->index == 0) // Only support JOINTS_0 for now
					jointsAccessor = attr->data;
				break;
			case cgltf_attribute_type_weights:
				if (attr->index == 0) // Only support WEIGHTS_0 for now
					weightsAccessor = attr->data;
				break;
			default:
				break;
			}
		}

		if (!positionAccessor)
		{
			log::error << L"Primitive missing POSITION attribute, skipping." << Endl;
			continue;
		}

		// Unpack attribute data
		AlignedVector< cgltf_float > positions, normals, tangents, colors, joints, weights;
		AlignedVector< cgltf_float > texCoords[8];

		if (!unpackAccessorData(positionAccessor, positions))
		{
			log::error << L"Failed to unpack position data." << Endl;
			continue;
		}

		if (normalAccessor)
			unpackAccessorData(normalAccessor, normals);

		if (tangentAccessor)
			unpackAccessorData(tangentAccessor, tangents);

		if (colorAccessor)
			unpackAccessorData(colorAccessor, colors);

		if (jointsAccessor)
			unpackAccessorData(jointsAccessor, joints);

		if (weightsAccessor)
			unpackAccessorData(weightsAccessor, weights);

		for (int i = 0; i < 8; ++i)
			if (texCoordAccessors[i])
				unpackAccessorData(texCoordAccessors[i], texCoords[i]);

		// Add positions to model
		const uint32_t vertexCount = positionAccessor->count;
		outModel.reservePositions(positionBase + vertexCount);

		for (uint32_t i = 0; i < vertexCount; ++i)
		{
			const cgltf_float* pos = &positions[i * 3];
			const Vector4 transformedPos = globalTransform * convertPosition(pos);
			outModel.addPosition(transformedPos);
		}

		// Convert joint vertex weights
		typedef SmallMap< uint32_t, float > bone_influences_t;
		AlignedVector< bone_influences_t > vertexJoints;
		vertexJoints.resize(vertexCount);

		if (jointsAccessor && weightsAccessor && joints.size() >= vertexCount * 4 && weights.size() >= vertexCount * 4)
		{
			for (uint32_t i = 0; i < vertexCount; ++i)
			{
				const cgltf_float* jointIndices = &joints[i * 4];
				const cgltf_float* jointWeights = &weights[i * 4];

				for (int j = 0; j < 4; ++j)
				{
					const uint32_t jointIndex = (uint32_t)jointIndices[j];
					const float jointWeight = jointWeights[j];

					if (jointWeight > FUZZY_EPSILON && jointIndex < outModel.getJointCount())
						vertexJoints[i].insert(std::pair< uint32_t, float >(jointIndex, jointWeight));
				}
			}
		}

		// Create vertices
		const uint32_t baseVertexIndex = outModel.getVertexCount();
		for (uint32_t i = 0; i < vertexCount; ++i)
		{
			Vertex vertex;
			vertex.setPosition(positionBase + i);

			// Set joint influences
			for (const auto& influence : vertexJoints[i])
				vertex.setJointInfluence(influence.first, influence.second);

			// Set normal
			if (normals.size() >= (i + 1) * 3)
			{
				const cgltf_float* norm = &normals[i * 3];
				vertex.setNormal(outModel.addUniqueNormal(
					globalNormalTransform * convertNormal(norm)));
			}

			// Set tangent
			if (tangents.size() >= (i + 1) * 4)
			{
				const cgltf_float* tang = &tangents[i * 4];
				vertex.setTangent(outModel.addUniqueNormal(
					globalNormalTransform * convertNormal(tang)));
				// Note: tangent[3] contains the bitangent sign, could be used for binormal calculation
			}

			// Set texture coordinates
			for (int texIndex = 0; texIndex < 8; ++texIndex)
			{
				if (texCoords[texIndex].size() >= (i + 1) * 2)
				{
					const cgltf_float* uv = &texCoords[texIndex][i * 2];
					// Load texture coordinates as-is (like Raylib does)
					// Don't flip V coordinate unless specifically needed by engine
					vertex.setTexCoord(texCoordChannels[texIndex], outModel.addUniqueTexCoord(Vector2(uv[0], uv[1])));
				}
			}

			// Set vertex color
			if (colors.size() >= (i + 1) * 3)
			{
				const cgltf_float* color = &colors[i * 3];
				cgltf_size colorComponents = colors.size() / vertexCount;
				vertex.setColor(outModel.addUniqueColor(
					convertColor(color, colorComponents)));
			}

			outModel.addVertex(vertex);
		}

		// Process indices and create polygons
		AlignedVector< uint32_t > indices;
		if (primitive->indices)
		{
			if (!unpackIndices(primitive->indices, indices))
			{
				log::error << L"Failed to unpack index data." << Endl;
				continue;
			}
		}
		else
		{
			// Generate indices for non-indexed geometry
			indices.resize(vertexCount);
			for (uint32_t i = 0; i < vertexCount; ++i)
				indices[i] = i;
		}

		// Create triangles
		const uint32_t triangleCount = indices.size() / 3;
		for (uint32_t i = 0; i < triangleCount; ++i)
		{
			Polygon& polygon = outModel.addPolygon();

			// Set material
			int32_t materialIndex = c_InvalidIndex;
			if (primitive->material)
			{
				cgltf_size gltfMaterialIndex = cgltf_material_index(data, primitive->material);
				auto it = materialMap.find(gltfMaterialIndex);
				if (it != materialMap.end())
					materialIndex = it->second;
			}
			polygon.setMaterial(materialIndex);

			// Add vertices in original order (both GLTF and engine are right-handed)
			polygon.addVertex(baseVertexIndex + indices[i * 3 + 0]);
			polygon.addVertex(baseVertexIndex + indices[i * 3 + 1]);
			polygon.addVertex(baseVertexIndex + indices[i * 3 + 2]);
		}
	}

	return true;
}

}
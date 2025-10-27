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
#include "Core/Misc/TString.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Formats/Gltf/GltfMaterialConverter.h"
#include "Model/Model.h"

#include <Core/Misc/String.h>

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

void getNodeTransform(cgltf_node* node, cgltf_float* outMatrix)
{
	cgltf_node_transform_world(node, outMatrix);
}

}

bool convertMesh(
	Model& outModel,
	cgltf_data* data,
	cgltf_node* meshNode,
	const Matrix44& axisTransform,
	const std::wstring& basePath)
{
	T_FATAL_ASSERT(meshNode->mesh != nullptr);

	cgltf_mesh* mesh = meshNode->mesh;

	// Get node's world transform
	cgltf_float nodeMatrix[16];
	getNodeTransform(meshNode, nodeMatrix);
	const Matrix44 Mnode = convertMatrix(nodeMatrix);
	const Matrix44 Mglobal = axisTransform * Mnode;
	const Matrix44 MglobalN = normalize(Mglobal);

	// Process each primitive (submesh) in the mesh
	for (cgltf_size primIdx = 0; primIdx < mesh->primitives_count; ++primIdx)
	{
		cgltf_primitive* primitive = &mesh->primitives[primIdx];

		// Only handle triangles
		if (primitive->type != cgltf_primitive_type_triangles)
		{
			log::warning << L"Skipping non-triangle primitive." << Endl;
			continue;
		}

		// Find accessors for different attributes
		cgltf_accessor* positionAccessor = nullptr;
		cgltf_accessor* normalAccessor = nullptr;
		cgltf_accessor* tangentAccessor = nullptr;
		cgltf_accessor* colorAccessor = nullptr;
		cgltf_accessor* jointsAccessor = nullptr;
		cgltf_accessor* weightsAccessor = nullptr;
		StaticVector< cgltf_accessor*, 8 > texcoordAccessors;
		StaticVector< std::wstring, 8 > texcoordNames;

		for (cgltf_size attrIdx = 0; attrIdx < primitive->attributes_count; ++attrIdx)
		{
			cgltf_attribute* attr = &primitive->attributes[attrIdx];
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
			case cgltf_attribute_type_color:
				colorAccessor = attr->data;
				break;
			case cgltf_attribute_type_joints:
				jointsAccessor = attr->data;
				break;
			case cgltf_attribute_type_weights:
				weightsAccessor = attr->data;
				break;
			case cgltf_attribute_type_texcoord:
				texcoordAccessors.push_back(attr->data);
				texcoordNames.push_back(L"UV" + toString(attr->index));
				break;
			default:
				break;
			}
		}

		if (!positionAccessor)
		{
			log::error << L"Mesh primitive missing position data." << Endl;
			return false;
		}

		// Add positions
		const uint32_t positionBase = uint32_t(outModel.getPositions().size());
		outModel.reservePositions(positionBase + uint32_t(positionAccessor->count));

		for (cgltf_size i = 0; i < positionAccessor->count; ++i)
		{
			cgltf_float position[3];
			cgltf_accessor_read_float(positionAccessor, i, position, 3);
			const Vector4 v = Mglobal * convertPosition(position);
			outModel.addPosition(v);
		}

		// Setup texture coordinate channels
		StaticVector< uint32_t, 8 > texCoordChannels;
		for (cgltf_size k = 0; k < texcoordAccessors.size(); ++k)
		{
			const uint32_t channel = outModel.addUniqueTexCoordChannel(texcoordNames[k]);
			texCoordChannels.push_back(channel);
		}

		// Convert materials
		SmallMap< int32_t, int32_t > materialMap;
		if (!convertMaterials(outModel, materialMap, data, primitive, basePath))
			return false;

		// Convert vertices based on indices or direct vertex order
		const uint32_t baseOutputVertexOffset = outModel.getVertexCount();

		if (primitive->indices)
		{
			// Indexed mesh
			cgltf_accessor* indexAccessor = primitive->indices;
			for (cgltf_size i = 0; i < indexAccessor->count; ++i)
			{
				cgltf_size vertexIndex = cgltf_accessor_read_index(indexAccessor, i);

				Vertex vertex;
				vertex.setPosition(positionBase + uint32_t(vertexIndex));

				// Joint influences
				if (jointsAccessor && weightsAccessor && vertexIndex < jointsAccessor->count)
				{
					cgltf_uint joints[4];
					cgltf_float weights[4];
					cgltf_accessor_read_uint(jointsAccessor, vertexIndex, joints, 4);
					cgltf_accessor_read_float(weightsAccessor, vertexIndex, weights, 4);

					for (int j = 0; j < 4; ++j)
						if (weights[j] > FUZZY_EPSILON)
							vertex.setJointInfluence(joints[j], weights[j]);
				}

				// Vertex color
				if (colorAccessor && vertexIndex < colorAccessor->count)
				{
					cgltf_float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
					cgltf_accessor_read_float(colorAccessor, vertexIndex, color, 4);
					vertex.setColor(outModel.addUniqueColor(convertColor4(color)));
				}

				// Texture coordinates
				for (cgltf_size k = 0; k < texcoordAccessors.size(); ++k)
				{
					if (vertexIndex < texcoordAccessors[k]->count)
					{
						cgltf_float texcoord[2];
						cgltf_accessor_read_float(texcoordAccessors[k], vertexIndex, texcoord, 2);
						const uint32_t channel = texCoordChannels[k];
						// Load texture coordinates as-is
						vertex.setTexCoord(channel, outModel.addUniqueTexCoord(convertVector2(texcoord)));
					}
				}

				// Normal
				if (normalAccessor && vertexIndex < normalAccessor->count)
				{
					cgltf_float normal[3];
					cgltf_accessor_read_float(normalAccessor, vertexIndex, normal, 3);
					vertex.setNormal(outModel.addUniqueNormal(
						(MglobalN * convertNormal(normal)).xyz0().normalized()));
				}

				// Tangent
				if (tangentAccessor && vertexIndex < tangentAccessor->count)
				{
					cgltf_float tangent[4];
					cgltf_accessor_read_float(tangentAccessor, vertexIndex, tangent, 4);
					vertex.setTangent(outModel.addUniqueNormal(
						(MglobalN * convertVector4(tangent)).xyz0().normalized()));
				}

				outModel.addVertex(vertex);
			}
		}
		else
		{
			// Non-indexed mesh
			for (cgltf_size i = 0; i < positionAccessor->count; ++i)
			{
				Vertex vertex;
				vertex.setPosition(positionBase + uint32_t(i));

				// Joint influences
				if (jointsAccessor && weightsAccessor && i < jointsAccessor->count)
				{
					cgltf_uint joints[4];
					cgltf_float weights[4];
					cgltf_accessor_read_uint(jointsAccessor, i, joints, 4);
					cgltf_accessor_read_float(weightsAccessor, i, weights, 4);

					for (int j = 0; j < 4; ++j)
						if (weights[j] > FUZZY_EPSILON)
							vertex.setJointInfluence(joints[j], weights[j]);
				}

				// Vertex color
				if (colorAccessor && i < colorAccessor->count)
				{
					cgltf_float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
					cgltf_accessor_read_float(colorAccessor, i, color, 4);
					vertex.setColor(outModel.addUniqueColor(convertColor4(color)));
				}

				// Texture coordinates
				for (cgltf_size k = 0; k < texcoordAccessors.size(); ++k)
				{
					if (i < texcoordAccessors[k]->count)
					{
						cgltf_float texcoord[2];
						cgltf_accessor_read_float(texcoordAccessors[k], i, texcoord, 2);
						const uint32_t channel = texCoordChannels[k];
						vertex.setTexCoord(channel, outModel.addUniqueTexCoord(convertVector2(texcoord)));
					}
				}

				// Normal
				if (normalAccessor && i < normalAccessor->count)
				{
					cgltf_float normal[3];
					cgltf_accessor_read_float(normalAccessor, i, normal, 3);
					vertex.setNormal(outModel.addUniqueNormal(
						(MglobalN * convertNormal(normal)).xyz0().normalized()));
				}

				// Tangent
				if (tangentAccessor && i < tangentAccessor->count)
				{
					cgltf_float tangent[4];
					cgltf_accessor_read_float(tangentAccessor, i, tangent, 4);
					vertex.setTangent(outModel.addUniqueNormal(
						(MglobalN * convertVector4(tangent)).xyz0().normalized()));
				}

				outModel.addVertex(vertex);
			}
		}

		// Create polygons (triangles)
		const uint32_t materialIndex = materialMap.find(0) != materialMap.end() ? materialMap[0] : c_InvalidIndex;
		const uint32_t vertexCount = primitive->indices ? uint32_t(primitive->indices->count) : uint32_t(positionAccessor->count);

		for (uint32_t i = 0; i < vertexCount; i += 3)
		{
			Polygon polygon;
			polygon.setMaterial(materialIndex);
			polygon.addVertex(baseOutputVertexOffset + i);
			polygon.addVertex(baseOutputVertexOffset + i + 1);
			polygon.addVertex(baseOutputVertexOffset + i + 2);
			outModel.addPolygon(polygon);
		}
	}

	return true;
}

}

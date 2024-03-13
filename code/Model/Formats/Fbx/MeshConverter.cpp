#pragma optimize( "", off )

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
	int32_t vertexId = 0;

//	FbxMesh* mesh = static_cast< FbxMesh* >(meshNode->GetNodeAttribute());
//	if (!mesh)
//		return false;

	// Convert materials.
	SmallMap< int32_t, int32_t > materialMap;
	if (!convertMaterials(outModel, materialMap, meshNode))
		return false;

	const uint32_t positionBase = uint32_t(outModel.getPositions().size());
//	const FbxAMatrix nodeTransform = meshNode->EvaluateGlobalTransform();

	const Matrix44 Mnode = convertMatrix(/*nodeTransform*/ meshNode->geometry_to_world);
	const Matrix44 Mglobal = axisTransform * Mnode;
	const Matrix44 MglobalN = normalize(Mglobal);

//	// Convert vertex positions.
//	FbxVector4* controlPoints = mesh->GetControlPoints();
//	if (!controlPoints)
//		return false;

//	const int32_t controlPointsCount = mesh->GetControlPointsCount();
//	outModel.reservePositions(positionBase + controlPointsCount);
	outModel.reservePositions(positionBase + meshNode->mesh->num_vertices);
	for (size_t i = 0; i <  meshNode->mesh->num_vertices; ++i)
	{
		const Vector4 v = Mglobal * convertPosition(meshNode->mesh->vertices[i]);
//		const Vector4 v = Mglobal * convertVector4(controlPoints[i]).xyz1();
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
		}
	}

//	const int32_t deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
//	for (int32_t i = 0; i < deformerCount; ++i)
//	{
//		FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
//		if (!skinDeformer)
//			continue;
//
//		const int32_t clusterCount = skinDeformer->GetClusterCount();
//		for (int32_t j = 0; j < clusterCount; ++j)
//		{
//			FbxCluster* cluster = skinDeformer->GetCluster(j);
//			if (!cluster)
//				continue;
//
//			const int32_t weightCount = cluster->GetControlPointIndicesCount();
//			if (weightCount <= 0)
//				continue;
//
//			const FbxNode* jointNode = cluster->GetLink();
//			T_ASSERT(jointNode);
//
//			std::wstring jointName = mbstows(jointNode->GetName());
//			const size_t p = jointName.find(L':');
//			if (p != std::wstring::npos)
//				jointName = jointName.substr(p + 1);
//
//			const uint32_t jointIndex = outModel.findJointIndex(jointName);
//			if (jointIndex == c_InvalidIndex)
//			{
//				log::warning << L"Unable to set vertex weight; no such joint \"" << jointName << L"\"." << Endl;
//				continue;
//			}
//
//			const double* weights = cluster->GetControlPointWeights();
//			const int32_t* indices = cluster->GetControlPointIndices();
//
//			for (int32_t k = 0; k < weightCount; ++k)
//			{
//				const int32_t vertexIndex = indices[k];
//				const float jointWeight = float(weights[k]);
//				if (jointWeight > FUZZY_EPSILON)
//					vertexJoints[vertexIndex].insert(std::pair< uint32_t, float >(jointIndex, jointWeight));
//			}
//		}
//	}

	// Convert polygons.
	//const int32_t polygonCount = mesh->GetPolygonCount();
	//for (int32_t i = 0; i < polygonCount; ++i)
	for (size_t i = 0; i < meshNode->mesh->num_faces; ++i)
	{
		const ufbx_face& face = meshNode->mesh->faces[i];

		//const int32_t polygonSize = mesh->GetPolygonSize(i);
		if (face.num_indices > Polygon::vertices_t::Capacity)
		{
			log::warning << L"Too many vertices (" << face.num_indices << L", max " << Polygon::vertices_t::Capacity << L") in polygon, skipped." << Endl;
			continue;
		}

		Polygon polygon;

		// Assign material; \fixme should probably create a polygon for each layer of material.
		//for (int32_t j = 0; j < mesh->GetLayerCount(); ++j)
		//{
		//	FbxLayerElementMaterial* layerMaterials = mesh->GetLayer(j)->GetMaterials();
		//	if (layerMaterials)
		//	{
		//		const int32_t materialIndex = layerMaterials->GetIndexArray().GetAt(i);
		//		polygon.setMaterial(materialMap[materialIndex]);
		//	}
		//}
		if (meshNode->mesh->face_material.count > 0)
			polygon.setMaterial(materialMap[meshNode->mesh->face_material[i]]);

		for (uint32_t j = 0; j < face.num_indices; ++j, ++vertexId)
		{
			//const int32_t pointIndex = mesh->GetPolygonVertex(i, j);
			const uint32_t vertexIndex = face.index_begin + j;
			const uint32_t pointIndex = meshNode->mesh->vertex_indices[vertexIndex];

			Vertex vertex;
			vertex.setPosition(positionBase + pointIndex);

			//if (pointIndex < int32_t(vertexJoints.size()))
			//{
			//	for (bone_influences_t::const_iterator k = vertexJoints[pointIndex].begin(); k != vertexJoints[pointIndex].end(); ++k)
			//		vertex.setJointInfluence(k->first, k->second);
			//}

			if (meshNode->mesh->vertex_color.exists)
			{
				if (meshNode->mesh->vertex_color.unique_per_vertex)
				{
					vertex.setColor(outModel.addUniqueColor(convertColor(
						meshNode->mesh->vertex_color.values.data[meshNode->mesh->vertex_color.indices.data[meshNode->mesh->vertex_first_index[vertexIndex]]]
					)));
				}
				else
				{
					vertex.setColor(outModel.addUniqueColor(convertColor(
						meshNode->mesh->vertex_color.values.data[meshNode->mesh->vertex_color.indices.data[vertexIndex]]
					)));
				}
			}

			for (size_t k = 0; k < meshNode->mesh->uv_sets.count; ++k)
			{
				const ufbx_uv_set& uvSet = meshNode->mesh->uv_sets.data[k];

				const std::wstring channelName = mbstows(uvSet.name.data);
				const uint32_t channel = outModel.addUniqueTexCoordChannel(channelName);

				if (uvSet.vertex_uv.exists)
				{
					if (uvSet.vertex_uv.unique_per_vertex)
					{
						vertex.setTexCoord(channel, outModel.addUniqueTexCoord(convertVector2(
							uvSet.vertex_uv.values.data[uvSet.vertex_uv.indices.data[meshNode->mesh->vertex_first_index[vertexIndex]]]
						)));
					}
					else
					{
						vertex.setTexCoord(channel, outModel.addUniqueTexCoord(convertVector2(
							uvSet.vertex_uv.values.data[uvSet.vertex_uv.indices.data[vertexIndex]]
						)));
					}
				}
			}

			if (meshNode->mesh->vertex_normal.exists)
			{
				if (meshNode->mesh->vertex_normal.unique_per_vertex)
				{
					vertex.setNormal(outModel.addUniqueNormal(MglobalN * convertNormal(
						meshNode->mesh->vertex_normal.values.data[meshNode->mesh->vertex_normal.indices.data[meshNode->mesh->vertex_first_index[vertexIndex]]]
					)));
				}
				else
				{
					vertex.setNormal(outModel.addUniqueNormal(MglobalN * convertNormal(
						meshNode->mesh->vertex_normal.values.data[meshNode->mesh->vertex_normal.indices.data[vertexIndex]]
					)));
				}
			}

			if (meshNode->mesh->vertex_tangent.exists)
			{
				if (meshNode->mesh->vertex_tangent.unique_per_vertex)
				{
					vertex.setTangent(outModel.addUniqueNormal(MglobalN * convertNormal(
						meshNode->mesh->vertex_tangent.values.data[meshNode->mesh->vertex_tangent.indices.data[meshNode->mesh->vertex_first_index[vertexIndex]]]
					)));
				}
				else
				{
					vertex.setTangent(outModel.addUniqueNormal(MglobalN * convertNormal(
						meshNode->mesh->vertex_tangent.values.data[meshNode->mesh->vertex_tangent.indices.data[vertexIndex]]
					)));
				}
			}

			if (meshNode->mesh->vertex_bitangent.exists)
			{
				if (meshNode->mesh->vertex_bitangent.unique_per_vertex)
				{
					vertex.setBinormal(outModel.addUniqueNormal(MglobalN * convertNormal(
						meshNode->mesh->vertex_bitangent.values.data[meshNode->mesh->vertex_bitangent.indices.data[meshNode->mesh->vertex_first_index[vertexIndex]]]
					)));
				}
				else
				{
					vertex.setBinormal(outModel.addUniqueNormal(MglobalN * convertNormal(
						meshNode->mesh->vertex_bitangent.values.data[meshNode->mesh->vertex_bitangent.indices.data[vertexIndex]]
					)));
				}
			}

			//for (int32_t k = mesh->GetLayerCount() - 1; k >= 0; --k)
			//{
			//	FbxLayerElementVertexColor* layerVertexColors = mesh->GetLayer(k)->GetVertexColors();
			//	if (layerVertexColors)
			//	{
			//		switch (layerVertexColors->GetMappingMode())
			//		{
			//		case FbxGeometryElement::eByControlPoint:
			//			switch (layerVertexColors->GetReferenceMode())
			//			{
			//			case FbxLayerElement::eDirect:
			//				{
			//					const FbxColor color = layerVertexColors->GetDirectArray().GetAt(pointIndex);
			//					vertex.setColor(outModel.addUniqueColor(convertColor(color)));
			//				}
			//				break;

			//			case FbxLayerElement::eIndexToDirect:
			//				{
			//					const int32_t id = layerVertexColors->GetIndexArray().GetAt(pointIndex);
			//					const FbxColor color = layerVertexColors->GetDirectArray().GetAt(id);
			//					vertex.setColor(outModel.addUniqueColor(convertColor(color)));
			//				}
			//				break;

			//			default:
			//				break;
			//			}
			//			break;

			//		case FbxGeometryElement::eByPolygonVertex:
			//			const int32_t textureUVIndex = mesh->GetPolygonVertexIndex(i) + j;
			//			switch (layerVertexColors->GetReferenceMode())
			//			{
			//			case FbxLayerElement::eDirect:
			//				{
			//					const FbxColor color = layerVertexColors->GetDirectArray().GetAt(textureUVIndex);
			//					vertex.setColor(outModel.addUniqueColor(convertColor(color)));
			//				}
			//				break;

			//			case FbxLayerElement::eIndexToDirect:
			//				{
			//					const int32_t id = layerVertexColors->GetIndexArray().GetAt(textureUVIndex);
			//					const FbxColor color = layerVertexColors->GetDirectArray().GetAt(id);
			//					vertex.setColor(outModel.addUniqueColor(convertColor(color)));
			//				}
			//				break;

			//			default:
			//				break;
			//			}
			//			break;
			//		}
			//	}

			//	// Vertex texture UVs.
			//	FbxLayerElementUV* layerUVs = mesh->GetLayer(k)->GetUVs();
			//	if (layerUVs)
			//	{
			//		const std::wstring channelName = mbstows(layerUVs->GetName());
			//		const uint32_t channel = outModel.addUniqueTexCoordChannel(channelName);

			//		switch (layerUVs->GetMappingMode())
			//		{
			//		case FbxLayerElement::eByControlPoint:
			//			switch (layerUVs->GetReferenceMode())
			//			{
			//			case FbxLayerElement::eDirect:
			//				{
			//					const Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(pointIndex)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
			//					vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
			//				}
			//				break;

			//			case FbxLayerElement::eIndexToDirect:
			//				{
			//					const int32_t id = layerUVs->GetIndexArray().GetAt(pointIndex);
			//					const Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(id)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
			//					vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
			//				}
			//				break;

			//			default:
			//				break;
			//			}
			//			break;

			//		case FbxLayerElement::eByPolygonVertex:
			//			{
			//				switch (layerUVs->GetReferenceMode())
			//				{
			//				case FbxLayerElement::eDirect:
			//					{
			//						const Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(vertexId)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
			//						vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
			//					}
			//					break;

			//				case FbxLayerElement::eIndexToDirect:
			//					{
			//						const int32_t id = layerUVs->GetIndexArray().GetAt(vertexId);
			//						const Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(id)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
			//						vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
			//					}
			//					break;

			//				default:
			//					break;
			//				}
			//			}
			//			break;

			//		default:
			//			break;
			//		}
			//	}

			//	// Vertex normals.
			//	FbxLayerElementNormal* layerNormals = mesh->GetLayer(k)->GetNormals();
			//	if (layerNormals)
			//	{
			//		if (layerNormals->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
			//		{
			//			switch (layerNormals->GetReferenceMode())
			//			{
			//			case FbxLayerElement::eDirect:
			//				{
			//					const FbxVector4 n = layerNormals->GetDirectArray().GetAt(vertexId);
			//					const Vector4 normal = MglobalN * convertNormal(n);
			//					vertex.setNormal(outModel.addUniqueNormal(normal.normalized()));
			//				}
			//				break;

			//			case FbxLayerElement::eIndexToDirect:
			//				{
			//					const int32_t id = layerNormals->GetIndexArray().GetAt(vertexId);
			//					const FbxVector4 n = layerNormals->GetDirectArray().GetAt(id);
			//					const Vector4 normal = MglobalN * convertNormal(n);
			//					vertex.setNormal(outModel.addUniqueNormal(normal.normalized()));
			//				}
			//				break;

			//			default:
			//				break;
			//			}
			//		}
			//	}

			//	// Vertex tangents.
			//	FbxLayerElementTangent* layerTangents = mesh->GetLayer(k)->GetTangents();
			//	if (layerTangents)
			//	{
			//		if (layerTangents->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
			//		{
			//			switch (layerTangents->GetReferenceMode())
			//			{
			//			case FbxLayerElement::eDirect:
			//				{
			//					const FbxVector4 t = layerTangents->GetDirectArray().GetAt(vertexId);
			//					const Vector4 tangent = MglobalN * convertNormal(t);
			//					vertex.setTangent(outModel.addUniqueNormal(tangent.normalized()));
			//				}
			//				break;

			//			case FbxLayerElement::eIndexToDirect:
			//				{
			//					const int32_t id = layerTangents->GetIndexArray().GetAt(vertexId);
			//					const FbxVector4 t = layerTangents->GetDirectArray().GetAt(id);
			//					const Vector4 tangent = MglobalN * convertNormal(t);
			//					vertex.setTangent(outModel.addUniqueNormal(tangent.normalized()));
			//				}
			//				break;

			//			default:
			//				break;
			//			}
			//		}
			//	}

			//	// Vertex binormals.
			//	FbxLayerElementBinormal* layerBinormals = mesh->GetLayer(k)->GetBinormals();
			//	if (layerBinormals)
			//	{
			//		if (layerBinormals->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
			//		{
			//			switch (layerBinormals->GetReferenceMode())
			//			{
			//			case FbxLayerElement::eDirect:
			//				{
			//					const FbxVector4 b = layerBinormals->GetDirectArray().GetAt(vertexId);
			//					const Vector4 binormal = MglobalN * convertNormal(b);
			//					vertex.setBinormal(outModel.addUniqueNormal(binormal.normalized()));
			//				}
			//				break;

			//			case FbxLayerElement::eIndexToDirect:
			//				{
			//					const int32_t id = layerBinormals->GetIndexArray().GetAt(vertexId);
			//					const FbxVector4 b = layerBinormals->GetDirectArray().GetAt(id);
			//					const Vector4 binormal = MglobalN * convertNormal(b);
			//					vertex.setBinormal(outModel.addUniqueNormal(binormal.normalized()));
			//				}
			//				break;

			//			default:
			//				break;
			//			}
			//		}
			//	}
			//}

			polygon.addVertex(outModel.addVertex(vertex));
		}

		outModel.addPolygon(polygon);
	}

//	fixMaterialUvSets(outModel);
	return true;
}

}

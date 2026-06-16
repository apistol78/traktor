/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Material.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Polygon.h"
#include "Model/Vertex.h"
#include "Model/Operations/CalculateNormals.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Animation.h"
#include "Model/Joint.h"
#include "Model/Pose.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Editor/CreateMeshFromGeometryTool.h"

namespace traktor::mcp
{
	namespace
	{

bool readVec(const Json* arr, int32_t n, float* out)
{
	if (!arr || !arr->isArray() || (int32_t)arr->size() < n)
		return false;
	for (int32_t i = 0; i < n; ++i)
		out[i] = (float)arr->at(i)->getReal();
	return true;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.CreateMeshFromGeometryTool", CreateMeshFromGeometryTool, IMcpTool)

CreateMeshFromGeometryTool::CreateMeshFromGeometryTool(editor::IEditor* editor)
:	m_editor(editor)
{
}

std::wstring CreateMeshFromGeometryTool::getName() const
{
	return L"create_mesh_from_geometry";
}

std::wstring CreateMeshFromGeometryTool::getDescription() const
{
	return L"Generate a mesh from raw geometry. \"positions\" is an array of [x,y,z] vertex positions; \"polygons\" is an array of polygons, each an array of indices into positions (clockwise winding for front faces; triangles, quads or n-gons). Optional \"normals\" and \"texCoords\" are per-position ([x,y,z] / [u,v], parallel to positions); smooth normals are computed when omitted. The model is triangulated (unless \"triangulate\"=false), written to a model file under the asset path (\"fileName\", e.g. \"Models/Foo.tmd\"; tmd/obj/gltf/fbx by extension) and a traktor.mesh.MeshAsset (Static) is created at \"path\". Optional \"maps\" binds material textures (keys \"diffuse\"/\"normal\"/\"roughness\"/\"specular\"/\"metalness\"/\"emissive\", each a texture instance guid; requires \"texCoords\") so the pipeline auto-generates a textured PBR material shader. Pass \"meshType\":\"skinned\" with \"joints\" (skeleton), \"jointIndices\" (per-position rigid skin weights) and \"animations\" (keyframed full poses) to author a skinned, animated mesh whose .tmd can drive a SkeletonAsset + AnimationAsset. Build it (build_asset) before use.";
}

Ref< Json > CreateMeshFromGeometryTool::getInputSchema() const
{
	auto str = [](const wchar_t* d) { Ref< Json > p = Json::createObject(); p->setString(L"type", L"string"); p->setString(L"description", d); return p; };
	auto arr = [](const wchar_t* d) { Ref< Json > p = Json::createObject(); p->setString(L"type", L"array"); p->setString(L"description", d); return p; };
	auto boolean = [](const wchar_t* d) { Ref< Json > p = Json::createObject(); p->setString(L"type", L"boolean"); p->setString(L"description", d); return p; };
	auto obj = [](const wchar_t* d) { Ref< Json > p = Json::createObject(); p->setString(L"type", L"object"); p->setString(L"description", d); return p; };

	Ref< Json > properties = Json::createObject();
	properties->set(L"path", str(L"Database path for the new MeshAsset (e.g. \"Models/Foo\")."));
	properties->set(L"fileName", str(L"Model file to write, relative to the asset path (e.g. \"Models/Foo.tmd\")."));
	properties->set(L"positions", arr(L"Array of [x,y,z] vertex positions."));
	properties->set(L"polygons", arr(L"Array of polygons; each is an array of indices into positions (CW)."));
	properties->set(L"normals", arr(L"Optional per-position [x,y,z] normals (parallel to positions)."));
	properties->set(L"texCoords", arr(L"Optional per-position [u,v] texture coordinates (parallel to positions)."));
	properties->set(L"material", str(L"Material name assigned to all polygons (default \"Default\")."));
	properties->set(L"triangulate", boolean(L"Triangulate the model (default true)."));
	properties->set(L"meshType", str(L"\"static\" (default) or \"skinned\". Skinned requires \"joints\" and \"jointIndices\"."));
	properties->set(L"joints", arr(L"Optional skeleton joints (order = joint index); each { name, parent (joint index or -1 for root), translation [x,y,z], rotation [x,y,z,w], length }."));
	properties->set(L"jointIndices", arr(L"Optional per-position joint index (parallel to positions) for rigid skinning (weight 1.0; -1 = none)."));
	properties->set(L"animations", arr(L"Optional animations; each { name, keyframes:[{ time, pose:[[tx,ty,tz,qx,qy,qz,qw], ...one full local transform per joint] }] }."));
	properties->set(L"maps", obj(L"Optional material texture maps; keys \"diffuse\"/\"normal\"/\"roughness\"/\"specular\"/\"metalness\"/\"emissive\", each a texture instance guid. Requires \"texCoords\"; the pipeline auto-generates a PBR material shader sampling them."));
	properties->set(L"guid", str(L"Optional explicit guid for the new instance."));

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"path"));
	required->push(Json::createString(L"fileName"));
	required->push(Json::createString(L"positions"));
	required->push(Json::createString(L"polygons"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > CreateMeshFromGeometryTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	// Recover any structured arguments a client delivered as JSON-encoded strings.
	Ref< Json > coerced = coerceStructuredArguments(arguments);
	arguments = coerced;

	const std::wstring path = (arguments && arguments->getMember(L"path")) ? arguments->getMember(L"path")->getString() : L"";
	const std::wstring fileName = (arguments && arguments->getMember(L"fileName")) ? arguments->getMember(L"fileName")->getString() : L"";
	const Json* positions = arguments ? arguments->getMember(L"positions") : nullptr;
	const Json* polygons = arguments ? arguments->getMember(L"polygons") : nullptr;
	if (path.empty() || fileName.empty())
	{
		outError = L"Missing \"path\" and/or \"fileName\".";
		return nullptr;
	}
	if (!positions || !positions->isArray() || positions->size() == 0)
	{
		outError = L"\"positions\" must be a non-empty array of [x,y,z].";
		return nullptr;
	}
	if (!polygons || !polygons->isArray() || polygons->size() == 0)
	{
		outError = L"\"polygons\" must be a non-empty array of index arrays.";
		return nullptr;
	}

	const uint32_t positionCount = positions->size();
	const Json* normals = arguments->getMember(L"normals");
	const Json* texCoords = arguments->getMember(L"texCoords");
	const bool haveNormals = normals && normals->isArray() && normals->size() == positionCount;
	const bool haveTexCoords = texCoords && texCoords->isArray() && texCoords->size() == positionCount;
	const std::wstring materialName = (arguments->getMember(L"material")) ? arguments->getMember(L"material")->getString() : L"Default";

	Ref< model::Model > mdl = new model::Model();

	uint32_t texCoordChannel = 0;
	if (haveTexCoords)
		texCoordChannel = mdl->addUniqueTexCoordChannel(L"UVMap");

	// Build the material, optionally binding texture maps so the mesh pipeline
	// auto-generates a textured PBR material shader from them.
	model::Material material(materialName);
	const Json* maps = arguments->getMember(L"maps");
	if (maps && maps->isObject())
	{
		if (!haveTexCoords)
		{
			outError = L"\"maps\" requires \"texCoords\" so the material can be sampled.";
			return nullptr;
		}
		auto bindMap = [&](const wchar_t* key) -> model::Material::Map {
			const Json* m = maps->getMember(key);
			if (!m)
				return model::Material::Map();
			const Guid g(m->getString());
			if (!g.isValid())
				return model::Material::Map();
			return model::Material::Map(key, texCoordChannel, false, g);
		};
		const model::Material::Map dm = bindMap(L"diffuse"); if (dm.texture.isNotNull()) material.setDiffuseMap(dm);
		const model::Material::Map nm = bindMap(L"normal"); if (nm.texture.isNotNull()) material.setNormalMap(nm);
		const model::Material::Map rm = bindMap(L"roughness"); if (rm.texture.isNotNull()) material.setRoughnessMap(rm);
		const model::Material::Map sm = bindMap(L"specular"); if (sm.texture.isNotNull()) material.setSpecularMap(sm);
		const model::Material::Map mm = bindMap(L"metalness"); if (mm.texture.isNotNull()) material.setMetalnessMap(mm);
		const model::Material::Map em = bindMap(L"emissive"); if (em.texture.isNotNull()) material.setEmissiveMap(em);
	}
	const uint32_t materialIndex = mdl->addMaterial(material);

	// Positions / normals / texcoords are kept index-parallel so a polygon's
	// position index also selects its normal and texcoord.
	AlignedVector< uint32_t > positionIndex(positionCount);
	AlignedVector< uint32_t > normalIndex(haveNormals ? positionCount : 0);
	AlignedVector< uint32_t > texCoordIndex(haveTexCoords ? positionCount : 0);

	for (uint32_t i = 0; i < positionCount; ++i)
	{
		float p[3];
		if (!readVec(positions->at(i), 3, p))
		{
			outError = L"positions[" + std::to_wstring(i) + L"] must be [x,y,z].";
			return nullptr;
		}
		positionIndex[i] = mdl->addPosition(Vector4(p[0], p[1], p[2], 1.0f));

		if (haveNormals)
		{
			float n[3];
			if (!readVec(normals->at(i), 3, n))
			{
				outError = L"normals[" + std::to_wstring(i) + L"] must be [x,y,z].";
				return nullptr;
			}
			normalIndex[i] = mdl->addNormal(Vector4(n[0], n[1], n[2], 0.0f).normalized());
		}
		if (haveTexCoords)
		{
			float t[2];
			if (!readVec(texCoords->at(i), 2, t))
			{
				outError = L"texCoords[" + std::to_wstring(i) + L"] must be [u,v].";
				return nullptr;
			}
			texCoordIndex[i] = mdl->addTexCoord(Vector2(t[0], t[1]));
		}
	}

	// Skeleton joints (optional): each { name, parent, translation[3], rotation[4], length }.
	const Json* jointsJson = arguments->getMember(L"joints");
	if (jointsJson && jointsJson->isArray())
	{
		for (uint32_t i = 0; i < jointsJson->size(); ++i)
		{
			const Json* jn = jointsJson->at(i);
			if (!jn || !jn->isObject())
			{
				outError = L"joints[" + std::to_wstring(i) + L"] must be an object.";
				return nullptr;
			}
			const std::wstring jname = jn->getMember(L"name") ? jn->getMember(L"name")->getString() : (L"joint" + std::to_wstring(i));
			const int32_t parent = jn->getMember(L"parent") ? (int32_t)jn->getMember(L"parent")->getNumber() : -1;
			float tr[3] = { 0.0f, 0.0f, 0.0f };
			if (jn->getMember(L"translation"))
				readVec(jn->getMember(L"translation"), 3, tr);
			float rot[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			if (jn->getMember(L"rotation"))
				readVec(jn->getMember(L"rotation"), 4, rot);
			const float length = jn->getMember(L"length") ? (float)jn->getMember(L"length")->getReal() : 0.1f;
			const uint32_t parentIdx = (parent >= 0) ? (uint32_t)parent : model::c_InvalidIndex;
			mdl->addJoint(model::Joint(parentIdx, jname, Transform(Vector4(tr[0], tr[1], tr[2], 0.0f), Quaternion(rot[0], rot[1], rot[2], rot[3])), length));
		}
	}

	// Per-position rigid skin weights (optional): joint index per position, weight 1.0.
	const Json* jointIndices = arguments->getMember(L"jointIndices");
	const bool haveJointWeights = jointIndices && jointIndices->isArray() && jointIndices->size() == positionCount;
	AlignedVector< int32_t > jointIndexPerPos;
	if (haveJointWeights)
	{
		jointIndexPerPos.resize(positionCount);
		for (uint32_t i = 0; i < positionCount; ++i)
			jointIndexPerPos[i] = (int32_t)jointIndices->at(i)->getNumber();
	}

	for (uint32_t i = 0; i < polygons->size(); ++i)
	{
		const Json* poly = polygons->at(i);
		if (!poly || !poly->isArray() || poly->size() < 3)
		{
			outError = L"polygons[" + std::to_wstring(i) + L"] must have at least 3 indices.";
			return nullptr;
		}
		model::Polygon polygon;
		polygon.setMaterial(materialIndex);
		for (uint32_t j = 0; j < poly->size(); ++j)
		{
			const int32_t pi = (int32_t)poly->at(j)->getNumber();
			if (pi < 0 || (uint32_t)pi >= positionCount)
			{
				outError = L"polygons[" + std::to_wstring(i) + L"] index " + std::to_wstring(pi) + L" out of range.";
				return nullptr;
			}
			model::Vertex vertex;
			vertex.setPosition(positionIndex[pi]);
			if (haveNormals)
				vertex.setNormal(normalIndex[pi]);
			if (haveTexCoords)
				vertex.setTexCoord(texCoordChannel, texCoordIndex[pi]);
			if (haveJointWeights && jointIndexPerPos[pi] >= 0)
				vertex.setJointInfluence((uint32_t)jointIndexPerPos[pi], 1.0f);
			polygon.addVertex(mdl->addVertex(vertex));
		}
		mdl->addPolygon(polygon);
	}

	if (!haveNormals)
		mdl->apply(model::CalculateNormals(true));

	const bool triangulate = !arguments->getMember(L"triangulate") || arguments->getMember(L"triangulate")->getBoolean();
	if (triangulate)
		mdl->apply(model::Triangulate());

	// Animations (optional): each { name, keyframes:[{ time, pose:[[tx,ty,tz,qx,qy,qz,qw] per joint] }] }.
	const Json* animations = arguments->getMember(L"animations");
	if (animations && animations->isArray())
	{
		const uint32_t jointCount = mdl->getJointCount();
		for (uint32_t a = 0; a < animations->size(); ++a)
		{
			const Json* an = animations->at(a);
			if (!an || !an->isObject())
				continue;
			Ref< model::Animation > anim = new model::Animation();
			anim->setName(an->getMember(L"name") ? an->getMember(L"name")->getString() : (L"anim" + std::to_wstring(a)));
			const Json* keyframes = an->getMember(L"keyframes");
			if (keyframes && keyframes->isArray())
			{
				for (uint32_t k = 0; k < keyframes->size(); ++k)
				{
					const Json* kf = keyframes->at(k);
					if (!kf || !kf->isObject())
						continue;
					const float time = kf->getMember(L"time") ? (float)kf->getMember(L"time")->getReal() : 0.0f;
					const Json* poseJson = kf->getMember(L"pose");
					Ref< model::Pose > pose = new model::Pose();
					if (poseJson && poseJson->isArray())
					{
						const uint32_t n = (poseJson->size() < jointCount) ? poseJson->size() : jointCount;
						for (uint32_t j = 0; j < n; ++j)
						{
							float p[7] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
							readVec(poseJson->at(j), 7, p);
							pose->setJointTransform(j, Transform(Vector4(p[0], p[1], p[2], 0.0f), Quaternion(p[3], p[4], p[5], p[6])));
						}
					}
					anim->insertKeyFrame(time, pose);
				}
			}
			mdl->addAnimation(anim);
		}
	}

	// Write the model to a file under the asset path.
	Ref< const PropertyGroup > settings = m_editor->getSettings();
	const std::wstring assetPath = settings ? settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"") : L"";
	if (assetPath.empty())
	{
		outError = L"Pipeline.AssetPath is not configured.";
		return nullptr;
	}
	const Path absolutePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath), Path(fileName));
	if (!FileSystem::getInstance().makeAllDirectories(absolutePath.getPathOnly()))
	{
		outError = L"Failed to create directory: " + absolutePath.getPathOnly();
		return nullptr;
	}
	if (!model::ModelFormat::writeAny(absolutePath, mdl))
	{
		outError = L"Failed to write model to " + absolutePath.getPathName() + L" (unsupported extension?).";
		return nullptr;
	}

	// Create the mesh asset referencing the written model.
	Ref< mesh::MeshAsset > meshAsset = new mesh::MeshAsset();
	meshAsset->setFileName(Path(fileName));
	const std::wstring meshTypeStr = arguments->getMember(L"meshType") ? arguments->getMember(L"meshType")->getString() : L"static";
	meshAsset->setMeshType(meshTypeStr == L"skinned" ? mesh::MeshAsset::MeshType::Skinned : mesh::MeshAsset::MeshType::Static);

	Guid guid;
	if (arguments->getMember(L"guid"))
	{
		const Guid g(arguments->getMember(L"guid")->getString());
		if (g.isValid())
			guid = g;
	}
	if (!guid.isValid())
		guid = Guid::create();

	Ref< db::Instance > instance = database->createInstance(path, db::CifReplaceExisting | db::CifKeepExistingGuid, &guid);
	if (!instance)
	{
		outError = L"Failed to create instance at \"" + path + L"\".";
		return nullptr;
	}
	if (!instance->setObject(meshAsset) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the new mesh asset.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", path);
	result->setString(L"fileName", fileName);
	result->setString(L"file", absolutePath.getPathName());
	result->set(L"positions", Json::createNumber((int64_t)positionCount));
	result->set(L"polygons", Json::createNumber((int64_t)polygons->size()));
	result->setBoolean(L"normalsComputed", !haveNormals);
	result->setBoolean(L"triangulated", triangulate);
	result->set(L"joints", Json::createNumber((int64_t)mdl->getJointCount()));
	result->set(L"animations", Json::createNumber((int64_t)mdl->getAnimationCount()));
	result->setString(L"meshType", meshTypeStr);
	result->setBoolean(L"committed", true);
	return result;
}

}

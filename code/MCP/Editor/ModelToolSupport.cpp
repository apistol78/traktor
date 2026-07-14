/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/ModelToolSupport.h"

#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "MCP/Editor/MeshAssetSupport.h"
#include "MCP/Editor/ModelSession.h"
#include "MCP/Editor/Json.h"
#include "Model/Animation.h"
#include "Model/IModelOperation.h"
#include "Model/Joint.h"
#include "Model/Material.h"
#include "Model/Model.h"
#include "Model/Polygon.h"
#include "Model/Pose.h"
#include "Model/Vertex.h"
#include "Model/Operations/BakeVertexColors.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateNormals.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Clear.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/NormalizeTexCoords.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/Unweld.h"
#include "Model/Operations/UnwrapUV.h"

namespace traktor::mcp
{
namespace
{

/*! Encode a model index, emitting the invalid sentinel as -1. */
Ref< Json > indexJson(uint32_t index)
{
	return Json::createNumber(index == model::c_InvalidIndex ? (int64_t)-1 : (int64_t)index);
}

/*! Decode a JSON index; a negative or absent value maps to the invalid sentinel. */
uint32_t jsonIndex(const Json* value, uint32_t defaultValue = model::c_InvalidIndex)
{
	if (!value || !value->isNumber())
		return defaultValue;
	const int64_t n = value->getNumber();
	return n < 0 ? model::c_InvalidIndex : (uint32_t)n;
}

double memberReal(const Json* obj, const wchar_t* key, double defaultValue)
{
	const Json* m = obj ? obj->getMember(key) : nullptr;
	return m ? m->getReal(defaultValue) : defaultValue;
}

bool memberBool(const Json* obj, const wchar_t* key, bool defaultValue)
{
	const Json* m = obj ? obj->getMember(key) : nullptr;
	return m ? m->getBoolean(defaultValue) : defaultValue;
}

int64_t memberInt(const Json* obj, const wchar_t* key, int64_t defaultValue)
{
	const Json* m = obj ? obj->getMember(key) : nullptr;
	return m ? m->getNumber(defaultValue) : defaultValue;
}

/*! Build a transform matrix from { translate, rotate, scale } or a raw 16-float "matrix". */
Matrix44 matrixFromJson(const Json* obj)
{
	const Json* matrix = obj ? obj->getMember(L"matrix") : nullptr;
	if (matrix && matrix->isArray() && matrix->size() >= 16)
	{
		float m[16];
		for (int32_t i = 0; i < 16; ++i)
			m[i] = (float)matrix->at(i)->getReal();
		return Matrix44::loadUnaligned(m);
	}

	Vector4 t = Vector4::zero();
	jsonToVec3(obj ? obj->getMember(L"translate") : nullptr, t, 1.0f);

	Vector4 r = Vector4::zero();
	Quaternion q = Quaternion::identity();
	if (jsonToVec4(obj ? obj->getMember(L"rotate") : nullptr, r))
		q = Quaternion(r.x(), r.y(), r.z(), r.w());

	Vector4 s = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
	const Json* scaleJson = obj ? obj->getMember(L"scale") : nullptr;
	if (scaleJson)
	{
		if (scaleJson->isArray() && scaleJson->size() >= 3)
			s = Vector4((float)scaleJson->at(0)->getReal(), (float)scaleJson->at(1)->getReal(), (float)scaleJson->at(2)->getReal(), 0.0f);
		else if (scaleJson->isNumber())
		{
			const float u = (float)scaleJson->getReal();
			s = Vector4(u, u, u, 0.0f);
		}
	}

	return traktor::Transform(t, q).toMatrix44() * traktor::scale(s);
}

/*! Bind an optional material texture map from a "maps" object entry. */
model::Material::Map mapFromJson(const Json* maps, const wchar_t* key)
{
	const Json* m = maps ? maps->getMember(key) : nullptr;
	if (!m)
		return model::Material::Map();
	if (m->isString())
		return model::Material::Map(key, 0, false, Guid(m->getString()));
	if (m->isObject())
	{
		const uint32_t channel = (uint32_t)memberInt(m, L"channel", 0);
		const bool anisotropic = memberBool(m, L"anisotropic", false);
		const std::wstring texture = m->getMember(L"texture") ? m->getMember(L"texture")->getString() : L"";
		return model::Material::Map(key, channel, anisotropic, Guid(texture));
	}
	return model::Material::Map();
}

}

model::Model* resolveModel(ModelSession* session, const Json* arguments, int32_t& outHandle, std::wstring& outError)
{
	outHandle = 0;
	const Json* h = arguments ? arguments->getMember(L"handle") : nullptr;
	if (!h || !h->isNumber())
	{
		outError = L"Missing integer \"handle\"; open a model with model_open first.";
		return nullptr;
	}
	const int32_t handle = (int32_t)h->getNumber();
	model::Model* model = session->get(handle);
	if (!model)
	{
		outError = L"No open model with handle " + std::to_wstring(handle) + L". Use model_open (or model_list to see open handles).";
		return nullptr;
	}
	outHandle = handle;
	return model;
}

Ref< Json > vec3ToJson(const Vector4& v)
{
	Ref< Json > arr = Json::createArray();
	arr->push(Json::createReal((float)v.x()));
	arr->push(Json::createReal((float)v.y()));
	arr->push(Json::createReal((float)v.z()));
	return arr;
}

Ref< Json > vec4ToJson(const Vector4& v)
{
	Ref< Json > arr = Json::createArray();
	arr->push(Json::createReal((float)v.x()));
	arr->push(Json::createReal((float)v.y()));
	arr->push(Json::createReal((float)v.z()));
	arr->push(Json::createReal((float)v.w()));
	return arr;
}

Ref< Json > vec2ToJson(const Vector2& v)
{
	Ref< Json > arr = Json::createArray();
	arr->push(Json::createReal((float)v.x));
	arr->push(Json::createReal((float)v.y));
	return arr;
}

bool jsonToVec3(const Json* arr, Vector4& out, float w)
{
	if (!arr || !arr->isArray() || arr->size() < 3)
		return false;
	out = Vector4((float)arr->at(0)->getReal(), (float)arr->at(1)->getReal(), (float)arr->at(2)->getReal(), w);
	return true;
}

bool jsonToVec4(const Json* arr, Vector4& out)
{
	if (!arr || !arr->isArray() || arr->size() < 4)
		return false;
	out = Vector4((float)arr->at(0)->getReal(), (float)arr->at(1)->getReal(), (float)arr->at(2)->getReal(), (float)arr->at(3)->getReal());
	return true;
}

bool jsonToVec2(const Json* arr, Vector2& out)
{
	if (!arr || !arr->isArray() || arr->size() < 2)
		return false;
	out = Vector2((float)arr->at(0)->getReal(), (float)arr->at(1)->getReal());
	return true;
}

Ref< Json > transformToJson(const Transform& transform)
{
	Ref< Json > obj = Json::createObject();
	obj->set(L"translation", vec3ToJson(transform.translation()));
	obj->set(L"rotation", vec4ToJson(transform.rotation().e));
	return obj;
}

Transform transformFromJson(const Json* obj)
{
	Vector4 t = Vector4::zero();
	jsonToVec3(obj ? obj->getMember(L"translation") : nullptr, t, 1.0f);

	Vector4 r = Vector4::zero();
	Quaternion q = Quaternion::identity();
	if (jsonToVec4(obj ? obj->getMember(L"rotation") : nullptr, r))
		q = Quaternion(r.x(), r.y(), r.z(), r.w());

	return Transform(t, q);
}

Ref< Json > materialToJson(const model::Material& material)
{
	Ref< Json > obj = Json::createObject();
	obj->setString(L"name", material.getName());

	const Color4f& c = material.getColor();
	Ref< Json > color = Json::createArray();
	color->push(Json::createReal((float)c.getRed()));
	color->push(Json::createReal((float)c.getGreen()));
	color->push(Json::createReal((float)c.getBlue()));
	color->push(Json::createReal((float)c.getAlpha()));
	obj->set(L"color", color);

	obj->set(L"diffuseTerm", Json::createReal(material.getDiffuseTerm()));
	obj->set(L"specularTerm", Json::createReal(material.getSpecularTerm()));
	obj->set(L"roughness", Json::createReal(material.getRoughness()));
	obj->set(L"metalness", Json::createReal(material.getMetalness()));
	obj->set(L"transparency", Json::createReal(material.getTransparency()));
	obj->set(L"emissive", Json::createReal(material.getEmissive()));
	obj->set(L"reflective", Json::createReal(material.getReflective()));
	obj->setBoolean(L"doubleSided", material.isDoubleSided());
	obj->set(L"blendOperator", Json::createNumber((int64_t)material.getBlendOperator()));

	// Report bound texture maps (name set) as { map: textureGuid }.
	Ref< Json > maps = Json::createObject();
	auto addMap = [&](const wchar_t* key, const model::Material::Map& m) {
		if (!m.name.empty())
			maps->setString(key, m.texture.format());
	};
	addMap(L"diffuse", material.getDiffuseMap());
	addMap(L"specular", material.getSpecularMap());
	addMap(L"roughness", material.getRoughnessMap());
	addMap(L"metalness", material.getMetalnessMap());
	addMap(L"transparency", material.getTransparencyMap());
	addMap(L"emissive", material.getEmissiveMap());
	addMap(L"reflective", material.getReflectiveMap());
	addMap(L"normal", material.getNormalMap());
	if (maps->getMemberCount() > 0)
		obj->set(L"maps", maps);

	return obj;
}

Ref< Json > jointToJson(const model::Joint& joint)
{
	Ref< Json > obj = transformToJson(joint.getTransform());
	obj->setString(L"name", joint.getName());
	obj->set(L"parent", indexJson(joint.getParent()));
	obj->set(L"length", Json::createReal(joint.getLength()));
	return obj;
}

Ref< Json > animationToJson(const model::Animation* animation)
{
	Ref< Json > obj = Json::createObject();
	if (!animation)
		return obj;
	obj->setString(L"name", animation->getName());
	const uint32_t count = animation->getKeyFrameCount();
	obj->set(L"keyFrameCount", Json::createNumber((int64_t)count));
	Ref< Json > times = Json::createArray();
	for (uint32_t i = 0; i < count; ++i)
		times->push(Json::createReal(animation->getKeyFrameTime(i)));
	obj->set(L"keyFrameTimes", times);
	obj->set(L"duration", Json::createReal(count > 0 ? animation->getKeyFrameTime(count - 1) : 0.0));
	return obj;
}

Ref< Json > polygonToJson(const model::Polygon& polygon)
{
	Ref< Json > obj = Json::createObject();
	obj->set(L"material", indexJson(polygon.getMaterial()));
	obj->set(L"normal", indexJson(polygon.getNormal()));
	obj->set(L"smoothGroup", Json::createNumber((int64_t)polygon.getSmoothGroup()));
	Ref< Json > vertices = Json::createArray();
	for (uint32_t i = 0; i < polygon.getVertexCount(); ++i)
		vertices->push(Json::createNumber((int64_t)polygon.getVertex(i)));
	obj->set(L"vertices", vertices);
	return obj;
}

Ref< Json > vertexToJson(const model::Vertex& vertex)
{
	Ref< Json > obj = Json::createObject();
	obj->set(L"position", indexJson(vertex.getPosition()));
	obj->set(L"color", indexJson(vertex.getColor()));
	obj->set(L"normal", indexJson(vertex.getNormal()));
	obj->set(L"tangent", indexJson(vertex.getTangent()));
	obj->set(L"binormal", indexJson(vertex.getBinormal()));

	Ref< Json > texCoords = Json::createArray();
	for (uint32_t i = 0; i < vertex.getTexCoordCount(); ++i)
		texCoords->push(indexJson(vertex.getTexCoord(i)));
	obj->set(L"texCoords", texCoords);

	Ref< Json > influences = Json::createArray();
	for (uint32_t i = 0; i < vertex.getJointInfluenceCount(); ++i)
		influences->push(Json::createReal(vertex.getJointInfluence(i)));
	obj->set(L"jointInfluences", influences);

	return obj;
}

model::Material materialFromJson(const Json* obj)
{
	const std::wstring name = (obj && obj->getMember(L"name")) ? obj->getMember(L"name")->getString() : L"Default";
	model::Material material(name);

	Vector4 c;
	if (jsonToVec4(obj ? obj->getMember(L"color") : nullptr, c))
		material.setColor(Color4f((float)c.x(), (float)c.y(), (float)c.z(), (float)c.w()));

	if (obj && obj->getMember(L"diffuseTerm")) material.setDiffuseTerm((float)obj->getMember(L"diffuseTerm")->getReal());
	if (obj && obj->getMember(L"specularTerm")) material.setSpecularTerm((float)obj->getMember(L"specularTerm")->getReal());
	if (obj && obj->getMember(L"roughness")) material.setRoughness((float)obj->getMember(L"roughness")->getReal());
	if (obj && obj->getMember(L"metalness")) material.setMetalness((float)obj->getMember(L"metalness")->getReal());
	if (obj && obj->getMember(L"transparency")) material.setTransparency((float)obj->getMember(L"transparency")->getReal());
	if (obj && obj->getMember(L"emissive")) material.setEmissive((float)obj->getMember(L"emissive")->getReal());
	if (obj && obj->getMember(L"reflective")) material.setReflective((float)obj->getMember(L"reflective")->getReal());
	if (obj && obj->getMember(L"doubleSided")) material.setDoubleSided(obj->getMember(L"doubleSided")->getBoolean());
	if (obj && obj->getMember(L"blendOperator")) material.setBlendOperator((model::Material::BlendOperator)obj->getMember(L"blendOperator")->getNumber());

	const Json* maps = obj ? obj->getMember(L"maps") : nullptr;
	if (maps && maps->isObject())
	{
		model::Material::Map m;
		if (!(m = mapFromJson(maps, L"diffuse")).name.empty()) material.setDiffuseMap(m);
		if (!(m = mapFromJson(maps, L"specular")).name.empty()) material.setSpecularMap(m);
		if (!(m = mapFromJson(maps, L"roughness")).name.empty()) material.setRoughnessMap(m);
		if (!(m = mapFromJson(maps, L"metalness")).name.empty()) material.setMetalnessMap(m);
		if (!(m = mapFromJson(maps, L"transparency")).name.empty()) material.setTransparencyMap(m);
		if (!(m = mapFromJson(maps, L"emissive")).name.empty()) material.setEmissiveMap(m);
		if (!(m = mapFromJson(maps, L"reflective")).name.empty()) material.setReflectiveMap(m);
		if (!(m = mapFromJson(maps, L"normal")).name.empty()) material.setNormalMap(m);
	}

	return material;
}

model::Joint jointFromJson(const Json* obj)
{
	const std::wstring name = (obj && obj->getMember(L"name")) ? obj->getMember(L"name")->getString() : L"joint";
	const uint32_t parent = jsonIndex(obj ? obj->getMember(L"parent") : nullptr);
	const float length = (float)memberReal(obj, L"length", 0.1);
	return model::Joint(parent, name, transformFromJson(obj), length);
}

model::Polygon polygonFromJson(const Json* obj)
{
	model::Polygon polygon;
	if (obj && obj->getMember(L"material"))
		polygon.setMaterial(jsonIndex(obj->getMember(L"material"), 0));
	if (obj && obj->getMember(L"normal"))
		polygon.setNormal(jsonIndex(obj->getMember(L"normal")));
	if (obj && obj->getMember(L"smoothGroup"))
		polygon.setSmoothGroup((uint32_t)obj->getMember(L"smoothGroup")->getNumber());
	const Json* vertices = obj ? obj->getMember(L"vertices") : nullptr;
	if (vertices && vertices->isArray())
	{
		for (uint32_t i = 0; i < vertices->size(); ++i)
			polygon.addVertex((uint32_t)vertices->at(i)->getNumber());
	}
	return polygon;
}

model::Vertex vertexFromJson(const Json* obj)
{
	model::Vertex vertex;
	vertex.setPosition(jsonIndex(obj ? obj->getMember(L"position") : nullptr, 0));
	if (obj && obj->getMember(L"color")) vertex.setColor(jsonIndex(obj->getMember(L"color")));
	if (obj && obj->getMember(L"normal")) vertex.setNormal(jsonIndex(obj->getMember(L"normal")));
	if (obj && obj->getMember(L"tangent")) vertex.setTangent(jsonIndex(obj->getMember(L"tangent")));
	if (obj && obj->getMember(L"binormal")) vertex.setBinormal(jsonIndex(obj->getMember(L"binormal")));

	// texCoords: array where the position in the array is the channel.
	const Json* texCoords = obj ? obj->getMember(L"texCoords") : nullptr;
	if (texCoords && texCoords->isArray())
	{
		for (uint32_t ch = 0; ch < texCoords->size(); ++ch)
			vertex.setTexCoord(ch, jsonIndex(texCoords->at(ch)));
	}

	// jointInfluences: array of { joint, weight }.
	const Json* influences = obj ? obj->getMember(L"jointInfluences") : nullptr;
	if (influences && influences->isArray())
	{
		for (uint32_t i = 0; i < influences->size(); ++i)
		{
			const Json* inf = influences->at(i);
			if (inf && inf->isObject())
				vertex.setJointInfluence((uint32_t)memberInt(inf, L"joint", 0), (float)memberReal(inf, L"weight", 0.0));
		}
	}

	return vertex;
}

Ref< model::Pose > poseFromJson(const Json* obj)
{
	Ref< model::Pose > pose = new model::Pose();
	const Json* jointTransforms = obj ? obj->getMember(L"jointTransforms") : nullptr;
	if (jointTransforms && jointTransforms->isArray())
	{
		for (uint32_t i = 0; i < jointTransforms->size(); ++i)
			pose->setJointTransform(i, transformFromJson(jointTransforms->at(i)));
	}
	return pose;
}

uint32_t clearFlagsFromJson(const Json* flagsArray, std::wstring& outError)
{
	if (!flagsArray || !flagsArray->isArray() || flagsArray->size() == 0)
		return model::Model::CfAll;

	uint32_t flags = 0;
	for (uint32_t i = 0; i < flagsArray->size(); ++i)
	{
		const std::wstring name = flagsArray->at(i)->getString();
		if (name == L"Materials") flags |= model::Model::CfMaterials;
		else if (name == L"Vertices") flags |= model::Model::CfVertices;
		else if (name == L"Polygons") flags |= model::Model::CfPolygons;
		else if (name == L"Positions") flags |= model::Model::CfPositions;
		else if (name == L"Colors") flags |= model::Model::CfColors;
		else if (name == L"Normals") flags |= model::Model::CfNormals;
		else if (name == L"TexCoords") flags |= model::Model::CfTexCoords;
		else if (name == L"Joints") flags |= model::Model::CfJoints;
		else if (name == L"All") flags |= model::Model::CfAll;
		else
		{
			outError = L"Unknown clear flag \"" + name + L"\". Valid: Materials, Vertices, Polygons, Positions, Colors, Normals, TexCoords, Joints, All.";
			return 0;
		}
	}
	return flags;
}

Ref< Json > modelCountsToJson(model::Model* model)
{
	Ref< Json > obj = Json::createObject();
	obj->set(L"materialCount", Json::createNumber((int64_t)model->getMaterialCount()));
	obj->set(L"vertexCount", Json::createNumber((int64_t)model->getVertexCount()));
	obj->set(L"polygonCount", Json::createNumber((int64_t)model->getPolygonCount()));
	obj->set(L"positionCount", Json::createNumber((int64_t)model->getPositionCount()));
	obj->set(L"colorCount", Json::createNumber((int64_t)model->getColorCount()));
	obj->set(L"normalCount", Json::createNumber((int64_t)model->getNormalCount()));
	obj->set(L"texCoordChannelCount", Json::createNumber((int64_t)model->getTexCoordChannels().size()));
	obj->set(L"jointCount", Json::createNumber((int64_t)model->getJointCount()));
	obj->set(L"animationCount", Json::createNumber((int64_t)model->getAnimationCount()));
	obj->set(L"blendTargetCount", Json::createNumber((int64_t)model->getBlendTargetCount()));

	// An empty model (no positions) has a degenerate bounding box whose
	// non-finite extents would serialize to invalid JSON; report null instead.
	const Aabb3 boundingBox = model->getBoundingBox();
	obj->set(L"boundingBox", boundingBox.empty() ? Json::createNull() : boundingBoxToJson(boundingBox));
	return obj;
}

Ref< Json > modelInspectToJson(model::Model* model)
{
	Ref< Json > obj = modelCountsToJson(model);

	Ref< Json > materials = Json::createArray();
	for (uint32_t i = 0; i < model->getMaterialCount(); ++i)
		materials->push(materialToJson(model->getMaterial(i)));
	obj->set(L"materials", materials);

	Ref< Json > joints = Json::createArray();
	for (uint32_t i = 0; i < model->getJointCount(); ++i)
		joints->push(jointToJson(model->getJoint(i)));
	obj->set(L"joints", joints);

	Ref< Json > animations = Json::createArray();
	for (uint32_t i = 0; i < model->getAnimationCount(); ++i)
		animations->push(animationToJson(model->getAnimation(i)));
	obj->set(L"animations", animations);

	Ref< Json > channels = Json::createArray();
	for (const auto& ch : model->getTexCoordChannels())
		channels->push(Json::createString(ch));
	obj->set(L"texCoordChannels", channels);

	Ref< Json > blendTargets = Json::createArray();
	for (uint32_t i = 0; i < model->getBlendTargetCount(); ++i)
		blendTargets->push(Json::createString(model->getBlendTarget(i)));
	obj->set(L"blendTargets", blendTargets);

	return obj;
}

Ref< model::IModelOperation > createOperation(const Json* opSpec, ModelSession* session, std::wstring& outError)
{
	if (!opSpec || !opSpec->isObject() || !opSpec->getMember(L"name"))
	{
		outError = L"Each operation must be an object with a \"name\".";
		return nullptr;
	}
	const std::wstring name = opSpec->getMember(L"name")->getString();

	if (name == L"Triangulate")
		return new model::Triangulate();
	if (name == L"CalculateNormals")
		return new model::CalculateNormals(memberBool(opSpec, L"replaceExisting", true));
	if (name == L"CalculateTangents")
		return new model::CalculateTangents(memberBool(opSpec, L"replaceExisting", true));
	if (name == L"CalculateConvexHull")
		return new model::CalculateConvexHull();
	if (name == L"CleanDegenerate")
		return new model::CleanDegenerate();
	if (name == L"CleanDuplicates")
		return new model::CleanDuplicates((float)memberReal(opSpec, L"positionDistance", 0.01));
	if (name == L"Clear")
	{
		const uint32_t flags = clearFlagsFromJson(opSpec->getMember(L"flags"), outError);
		if (!outError.empty())
			return nullptr;
		return new model::Clear(flags);
	}
	if (name == L"CullDistantFaces")
	{
		const Json* region = opSpec->getMember(L"region");
		Vector4 mn = Vector4::zero(), mx = Vector4::zero();
		if (!region || !jsonToVec3(region->getMember(L"min"), mn, 1.0f) || !jsonToVec3(region->getMember(L"max"), mx, 1.0f))
		{
			outError = L"CullDistantFaces requires \"region\": { \"min\": [x,y,z], \"max\": [x,y,z] }.";
			return nullptr;
		}
		return new model::CullDistantFaces(Aabb3(mn, mx));
	}
	if (name == L"FlattenDoubleSided")
		return new model::FlattenDoubleSided();
	if (name == L"MergeCoplanarAdjacents")
		return new model::MergeCoplanarAdjacents();
	if (name == L"MergeModel")
	{
		const int32_t sourceHandle = (int32_t)memberInt(opSpec, L"modelHandle", 0);
		model::Model* source = session ? session->get(sourceHandle) : nullptr;
		if (!source)
		{
			outError = L"MergeModel requires \"modelHandle\" referencing an open source model.";
			return nullptr;
		}
		const Matrix44 m = matrixFromJson(opSpec->getMember(L"transform"));
		return new model::MergeModel(*source, traktor::Transform(m), (float)memberReal(opSpec, L"positionDistance", 0.01));
	}
	if (name == L"NormalizeTexCoords")
	{
		return new model::NormalizeTexCoords(
			(uint32_t)memberInt(opSpec, L"channel", 0),
			(float)memberReal(opSpec, L"marginU", 0.0),
			(float)memberReal(opSpec, L"marginV", 0.0),
			(float)memberReal(opSpec, L"stepU", 0.0),
			(float)memberReal(opSpec, L"stepV", 0.0));
	}
	if (name == L"Quantize")
	{
		const Json* step = opSpec->getMember(L"step");
		if (step && step->isArray() && step->size() >= 3)
			return new model::Quantize(Vector4((float)step->at(0)->getReal(), (float)step->at(1)->getReal(), (float)step->at(2)->getReal(), 0.0f));
		return new model::Quantize((float)memberReal(opSpec, L"step", 0.01));
	}
	if (name == L"Reduce")
		return new model::Reduce((float)memberReal(opSpec, L"target", 0.5));
	if (name == L"ScaleAlongNormal")
		return new model::ScaleAlongNormal((float)memberReal(opSpec, L"distance", 0.0));
	if (name == L"SortCacheCoherency")
		return new model::SortCacheCoherency();
	if (name == L"SortProjectedArea")
		return new model::SortProjectedArea(memberBool(opSpec, L"insideOut", false));
	if (name == L"Transform")
		return new model::Transform(matrixFromJson(opSpec));
	if (name == L"Unweld")
		return new model::Unweld();
	if (name == L"UnwrapUV")
		return new model::UnwrapUV((int32_t)memberInt(opSpec, L"channel", 0), (uint32_t)memberInt(opSpec, L"textureSize", 1024));
	if (name == L"BakeVertexColors")
		return new model::BakeVertexColors();
	if (name == L"Boolean")
	{
		const int32_t handleA = (int32_t)memberInt(opSpec, L"modelHandleA", 0);
		const int32_t handleB = (int32_t)memberInt(opSpec, L"modelHandleB", 0);
		model::Model* a = session ? session->get(handleA) : nullptr;
		model::Model* b = session ? session->get(handleB) : nullptr;
		if (!a || !b)
		{
			outError = L"Boolean requires \"modelHandleA\" and \"modelHandleB\" referencing open models (the operation writes its result into the invoked handle, which must be a different, e.g. blank, model).";
			return nullptr;
		}
		const std::wstring op = opSpec->getMember(L"operation") ? opSpec->getMember(L"operation")->getString() : L"union";
		model::Boolean::BooleanOperation bop = model::Boolean::BoUnion;
		if (op == L"intersection") bop = model::Boolean::BoIntersection;
		else if (op == L"difference") bop = model::Boolean::BoDifference;
		else if (op != L"union")
		{
			outError = L"Boolean \"operation\" must be one of: union, intersection, difference.";
			return nullptr;
		}
		return new model::Boolean(*a, traktor::Transform(matrixFromJson(opSpec->getMember(L"transformA"))), *b, traktor::Transform(matrixFromJson(opSpec->getMember(L"transformB"))), bop);
	}

	outError = L"Unknown operation \"" + name + L"\". Valid names: Triangulate, CalculateNormals, CalculateTangents, CalculateConvexHull, CleanDegenerate, CleanDuplicates, Clear, CullDistantFaces, FlattenDoubleSided, MergeCoplanarAdjacents, MergeModel, NormalizeTexCoords, Quantize, Reduce, ScaleAlongNormal, SortCacheCoherency, SortProjectedArea, Transform, Unweld, UnwrapUV, BakeVertexColors, Boolean.";
	return nullptr;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Guid.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Mesh/Editor/MeshAsset.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/MeshAssetSupport.h"

namespace traktor::mcp
{
	namespace
	{

const wchar_t* const c_meshAssetTypeName = L"traktor.mesh.MeshAsset";

/*! Position of the asset origin (0,0,0) within the box, per axis (0=min, 1=max).
 * Falls back to 0.5 on a degenerate (zero-size) axis. */
void originNormalized(const Aabb3& bbox, float& outX, float& outY, float& outZ)
{
	const Vector4 size = bbox.mx - bbox.mn;
	const float sx = (float)size.x();
	const float sy = (float)size.y();
	const float sz = (float)size.z();
	outX = sx > 1e-4f ? (0.0f - (float)bbox.mn.x()) / sx : 0.5f;
	outY = sy > 1e-4f ? (0.0f - (float)bbox.mn.y()) / sy : 0.5f;
	outZ = sz > 1e-4f ? (0.0f - (float)bbox.mn.z()) / sz : 0.5f;
}

Ref< Json > vec3ToJson(float x, float y, float z)
{
	Ref< Json > arr = Json::createArray();
	arr->push(Json::createReal((double)x));
	arr->push(Json::createReal((double)y));
	arr->push(Json::createReal((double)z));
	return arr;
}

	}

mesh::MeshAsset* loadMeshAsset(db::Instance* instance, Ref< ISerializable >& outObject, std::wstring& outError)
{
	if (instance->getPrimaryTypeName() != c_meshAssetTypeName)
	{
		outError = L"Instance is not a MeshAsset (primary type: " + instance->getPrimaryTypeName() + L").";
		return nullptr;
	}

	outObject = instance->getObject();
	if (!outObject)
	{
		outError = L"Failed to read mesh asset object.";
		return nullptr;
	}

	ISerializable* object = outObject;
	return static_cast< mesh::MeshAsset* >(object);
}

Ref< Json > materialMapToJson(const SmallMap< std::wstring, Guid >& map)
{
	Ref< Json > obj = Json::createObject();
	for (const auto& it : map)
		obj->setString(it.first, it.second.format());
	return obj;
}

std::wstring meshTypeName(mesh::MeshAsset::MeshType type)
{
	switch (type)
	{
	case mesh::MeshAsset::MeshType::Instance: return L"Instance";
	case mesh::MeshAsset::MeshType::Skinned: return L"Skinned";
	case mesh::MeshAsset::MeshType::Static: return L"Static";
	default: return L"Unknown";
	}
}

Ref< Json > meshTargetProperties()
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the mesh asset instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the mesh asset instance (alternative to guid).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	return properties;
}

std::wstring pivotLabel(const Aabb3& bbox)
{
	float nx, ny, nz;
	originNormalized(bbox, nx, ny, nz);

	const std::wstring vertical = ny < 0.2f ? L"base" : (ny > 0.8f ? L"top" : L"mid-height");
	const bool horizontallyCentered = (nx > 0.4f && nx < 0.6f && nz > 0.4f && nz < 0.6f);
	return vertical + (horizontallyCentered ? L", horizontally centered" : L", horizontally offset");
}

Ref< Json > boundingBoxToJson(const Aabb3& bbox)
{
	const Vector4 mn = bbox.mn;
	const Vector4 mx = bbox.mx;
	const Vector4 size = mx - mn;
	const Vector4 center = bbox.getCenter();

	float nx, ny, nz;
	originNormalized(bbox, nx, ny, nz);

	Ref< Json > pivot = Json::createObject();
	pivot->set(L"normalized", vec3ToJson(nx, ny, nz));
	pivot->setString(L"label", pivotLabel(bbox));

	Ref< Json > result = Json::createObject();
	result->set(L"size", vec3ToJson((float)size.x(), (float)size.y(), (float)size.z()));
	result->set(L"min", vec3ToJson((float)mn.x(), (float)mn.y(), (float)mn.z()));
	result->set(L"max", vec3ToJson((float)mx.x(), (float)mx.y(), (float)mx.z()));
	result->set(L"center", vec3ToJson((float)center.x(), (float)center.y(), (float)center.z()));
	result->set(L"pivot", pivot);
	result->setString(L"space", L"model units (source mesh space, before asset scale/offset)");
	return result;
}

}

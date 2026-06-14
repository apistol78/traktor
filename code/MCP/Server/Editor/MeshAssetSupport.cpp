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

}

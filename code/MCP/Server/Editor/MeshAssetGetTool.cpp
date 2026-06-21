/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Aabb3.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetRasterizer.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Editor/MeshAssetGetTool.h"
#include "MCP/Server/Editor/MeshAssetSupport.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.MeshAssetGetTool", MeshAssetGetTool, IMcpTool)

MeshAssetGetTool::MeshAssetGetTool(editor::IEditor* editor)
:	m_editor(editor)
{
}

std::wstring MeshAssetGetTool::getName() const
{
	return L"get_mesh_asset";
}

std::wstring MeshAssetGetTool::getDescription() const
{
	return L"Read a mesh asset's material slot bindings. Returns the materialShaders (material slot name -> shader graph guid) and materialTextures maps, plus the mesh type. Use this to inspect which slots have an explicit material shader assigned.";
}

Ref< Json > MeshAssetGetTool::getInputSchema() const
{
	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", meshTargetProperties());
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > MeshAssetGetTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	Ref< db::Instance > instance = resolveInstance(database, arguments, outError);
	if (!instance)
	{
		if (outError.empty())
			outError = L"No mesh asset instance found for the given guid/path.";
		return nullptr;
	}

	Ref< ISerializable > object;
	mesh::MeshAsset* meshAsset = loadMeshAsset(instance, object, outError);
	if (!meshAsset)
		return nullptr;

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"name", instance->getName());
	result->setString(L"meshType", meshTypeName(meshAsset->getMeshType()));
	result->set(L"materialShaders", materialMapToJson(meshAsset->getMaterialShaders()));
	result->set(L"materialTextures", materialMapToJson(meshAsset->getMaterialTextures()));

	// Source-mesh footprint: dimensions + pivot, for layout/kit-bashing reasoning.
	// Omitted silently if the model file cannot be read (this stays a metadata read).
	Aabb3 boundingBox;
	if (mesh::MeshAssetRasterizer().getBoundingBox(m_editor, meshAsset, boundingBox))
		result->set(L"boundingBox", boundingBoxToJson(boundingBox));

	return result;
}

}

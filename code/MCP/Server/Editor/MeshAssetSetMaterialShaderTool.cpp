/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/MeshAssetSetMaterialShaderTool.h"

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Editor/MeshAssetSupport.h"
#include "MCP/Server/Json.h"
#include "Mesh/Editor/MeshAsset.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.MeshAssetSetMaterialShaderTool", MeshAssetSetMaterialShaderTool, IMcpTool)

MeshAssetSetMaterialShaderTool::MeshAssetSetMaterialShaderTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring MeshAssetSetMaterialShaderTool::getName() const
{
	return L"set_mesh_material_shader";
}

std::wstring MeshAssetSetMaterialShaderTool::getDescription() const
{
	return L"Assign (or clear) the shader graph bound to a material slot on a mesh asset. Sets materialShaders[material] = shader guid and commits the change to the source database; pass an empty \"shader\" to remove the binding. The shader must reference an existing ShaderGraph instance.";
}

Ref< Json > MeshAssetSetMaterialShaderTool::getInputSchema() const
{
	Ref< Json > materialProperty = Json::createObject();
	materialProperty->setString(L"type", L"string");
	materialProperty->setString(L"description", L"Name of the material slot to bind (as defined by the source mesh, e.g. \"Vanguard_VisorMat\").");

	Ref< Json > shaderProperty = Json::createObject();
	shaderProperty->setString(L"type", L"string");
	shaderProperty->setString(L"description", L"Guid of the ShaderGraph to bind to the slot. Pass an empty string to remove the binding.");

	Ref< Json > properties = meshTargetProperties();
	properties->set(L"material", materialProperty);
	properties->set(L"shader", shaderProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"material"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > MeshAssetSetMaterialShaderTool::invoke(const Json* arguments, std::wstring& outError)
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

	const std::wstring material = (arguments && arguments->getMember(L"material")) ? arguments->getMember(L"material")->getString() : L"";
	if (material.empty())
	{
		outError = L"Missing \"material\" (the material slot name).";
		return nullptr;
	}

	const std::wstring shaderText = (arguments && arguments->getMember(L"shader")) ? arguments->getMember(L"shader")->getString() : L"";
	const bool clear = shaderText.empty();

	Guid shaderGuid;
	if (!clear)
	{
		shaderGuid = Guid(shaderText);
		if (!shaderGuid.isValid())
		{
			outError = L"Invalid shader guid: " + shaderText;
			return nullptr;
		}
		Ref< db::Instance > shaderInstance = database->getInstance(shaderGuid);
		if (!shaderInstance)
		{
			outError = L"Shader instance not found: " + shaderText;
			return nullptr;
		}
		if (shaderInstance->getPrimaryTypeName().find(L"ShaderGraph") == std::wstring::npos)
		{
			outError = L"Target shader is not a ShaderGraph (primary type: " + shaderInstance->getPrimaryTypeName() + L").";
			return nullptr;
		}
	}

	Ref< ISerializable > object;
	mesh::MeshAsset* meshAsset = loadMeshAsset(instance, object, outError);
	if (!meshAsset)
		return nullptr;

	SmallMap< std::wstring, Guid > shaders = meshAsset->getMaterialShaders();
	const bool existed = (shaders.find(material) != shaders.end());

	if (clear)
		shaders.remove(material);
	else
		shaders[material] = shaderGuid;

	meshAsset->setMaterialShaders(shaders);

	if (!instance->checkout())
	{
		outError = L"Failed to checkout instance for writing.";
		return nullptr;
	}
	if (!instance->setObject(meshAsset) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the updated mesh asset.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"material", material);
	if (clear)
		result->setString(L"action", existed ? L"cleared" : L"noop");
	else
	{
		result->setString(L"action", existed ? L"updated" : L"added");
		result->setString(L"shader", shaderGuid.format());
	}
	result->setBoolean(L"committed", true);
	result->set(L"materialShaders", materialMapToJson(meshAsset->getMaterialShaders()));
	return result;
}

}

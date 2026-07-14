/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ModelSaveTool.h"

#include "Core/Guid.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/McpToolSupport.h"
#include "MCP/Editor/ModelSession.h"
#include "MCP/Editor/ModelToolSupport.h"
#include "MCP/Editor/Json.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"

namespace traktor::mcp
{
namespace
{

bool writeModelFile(const Path& absolutePath, const model::Model* model, std::wstring& outError)
{
	if (!FileSystem::getInstance().makeAllDirectories(absolutePath.getPathOnly()))
	{
		outError = L"Failed to create directory: " + absolutePath.getPathOnly();
		return false;
	}
	if (!model::ModelFormat::writeAny(absolutePath, model))
	{
		outError = L"Failed to write model to \"" + absolutePath.getPathName() + L"\" (unsupported extension?).";
		return false;
	}
	return true;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelSaveTool", ModelSaveTool, IMcpTool)

ModelSaveTool::ModelSaveTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelSaveTool::getName() const
{
	return L"model_save";
}

std::wstring ModelSaveTool::getDescription() const
{
	return L"Persist an open model (by \"handle\"). Provide exactly one target: \"file\" (write to a model file path via ModelFormat; format chosen by extension - tmd/obj/gltf/fbx/...; absolute or relative to Pipeline.AssetPath); \"overwriteSource\":true (write back to the exact file the model was opened from - only valid for file/meshAsset sources); or \"meshAsset\":{ \"path\": <db path>, \"fileName\": <path relative to Pipeline.AssetPath, e.g. \"Models/Foo.tmd\">, \"guid\"?, \"meshType\"? (\"static\"|\"skinned\") } which writes the model file then creates/updates a traktor.mesh.MeshAsset instance referencing it (build it with build_asset before use). Returns { handle, file, guid?, path? }.";
}

Ref< Json > ModelSaveTool::getInputSchema() const
{
	auto str = [](const wchar_t* d) {
		Ref< Json > p = Json::createObject();
		p->setString(L"type", L"string");
		p->setString(L"description", d);
		return p;
	};

	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of an open model.");

	Ref< Json > overwrite = Json::createObject();
	overwrite->setString(L"type", L"boolean");
	overwrite->setString(L"description", L"Write back to the file the model was opened from.");

	Ref< Json > meshAsset = Json::createObject();
	meshAsset->setString(L"type", L"object");
	meshAsset->setString(L"description", L"Write the model file and create/update a MeshAsset: { path, fileName, guid?, meshType? }.");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);
	properties->set(L"file", str(L"Model file path to write (absolute or relative to Pipeline.AssetPath)."));
	properties->set(L"overwriteSource", overwrite);
	properties->set(L"meshAsset", meshAsset);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"handle"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ModelSaveTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< Json > coerced = coerceStructuredArguments(arguments);
	arguments = coerced;

	int32_t handle = 0;
	model::Model* model = resolveModel(m_session, arguments, handle, outError);
	if (!model)
		return nullptr;

	Ref< const PropertyGroup > settings = m_editor->getSettings();
	const std::wstring assetPath = settings ? settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"") : L"";

	const std::wstring fileStr = arguments->getMember(L"file") ? arguments->getMember(L"file")->getString() : L"";
	const bool overwriteSource = arguments->getMember(L"overwriteSource") && arguments->getMember(L"overwriteSource")->getBoolean();
	const Json* meshAssetJson = arguments->getMember(L"meshAsset");

	// Target: overwrite the opened source file.
	if (overwriteSource)
	{
		const ModelSession::Entry* entry = m_session->entry(handle);
		if (!entry || entry->filePath.empty())
		{
			outError = L"\"overwriteSource\" is only valid for models opened from a file or MeshAsset.";
			return nullptr;
		}
		if (!writeModelFile(Path(entry->filePath), model, outError))
			return nullptr;
		Ref< Json > result = Json::createObject();
		result->set(L"handle", Json::createNumber((int64_t)handle));
		result->setString(L"file", entry->filePath);
		return result;
	}

	// Target: create/update a MeshAsset referencing a written model file.
	if (meshAssetJson && meshAssetJson->isObject())
	{
		const std::wstring path = meshAssetJson->getMember(L"path") ? meshAssetJson->getMember(L"path")->getString() : L"";
		const std::wstring fileName = meshAssetJson->getMember(L"fileName") ? meshAssetJson->getMember(L"fileName")->getString() : L"";
		if (path.empty() || fileName.empty())
		{
			outError = L"\"meshAsset\" requires \"path\" and \"fileName\".";
			return nullptr;
		}
		if (assetPath.empty())
		{
			outError = L"Pipeline.AssetPath is not configured.";
			return nullptr;
		}
		Ref< db::Database > database = m_editor->getSourceDatabase();
		if (!database)
		{
			outError = L"No source database is currently open.";
			return nullptr;
		}

		const Path absolutePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath), Path(fileName));
		if (!writeModelFile(absolutePath, model, outError))
			return nullptr;

		Ref< mesh::MeshAsset > meshAsset = new mesh::MeshAsset();
		meshAsset->setFileName(Path(fileName));
		const std::wstring meshType = meshAssetJson->getMember(L"meshType") ? meshAssetJson->getMember(L"meshType")->getString() : L"static";
		meshAsset->setMeshType(meshType == L"skinned" ? mesh::MeshAsset::MeshType::Skinned : mesh::MeshAsset::MeshType::Static);

		Guid guid;
		if (meshAssetJson->getMember(L"guid"))
		{
			const Guid g(meshAssetJson->getMember(L"guid")->getString());
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
		result->set(L"handle", Json::createNumber((int64_t)handle));
		result->setString(L"file", absolutePath.getPathName());
		result->setString(L"fileName", fileName);
		result->setString(L"guid", instance->getGuid().format());
		result->setString(L"path", path);
		return result;
	}

	// Target: write to an explicit file path.
	if (!fileStr.empty())
	{
		const Path absolutePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath), Path(fileStr));
		if (!writeModelFile(absolutePath, model, outError))
			return nullptr;
		Ref< Json > result = Json::createObject();
		result->set(L"handle", Json::createNumber((int64_t)handle));
		result->setString(L"file", absolutePath.getPathName());
		return result;
	}

	outError = L"Provide one of \"file\", \"overwriteSource\":true, or \"meshAsset\":{...}.";
	return nullptr;
}

}

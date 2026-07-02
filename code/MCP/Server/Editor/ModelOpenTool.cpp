/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelOpenTool.h"

#include "Core/Guid.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Editor/MeshAssetSupport.h"
#include "MCP/Server/Editor/ModelSession.h"
#include "MCP/Server/Editor/ModelToolSupport.h"
#include "MCP/Server/Json.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelOpenTool", ModelOpenTool, IMcpTool)

ModelOpenTool::ModelOpenTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelOpenTool::getName() const
{
	return L"model_open";
}

std::wstring ModelOpenTool::getDescription() const
{
	return L"Open a working model into the model session and return an integer \"handle\" the other model_* tools operate on. Provide exactly one source: \"file\" (a model file path read via ModelFormat - obj/fbx/gltf/lwo/tmd/...; absolute, or relative to Pipeline.AssetPath; optional \"filter\"), \"meshAsset\" (a traktor.mesh.MeshAsset instance guid or database path, whose referenced source file is read), or \"blank\":true to create an empty model from scratch (then build it up with model_edit and persist with model_save). Returns { handle, source, summary } where summary holds element counts and the bounding box. The model lives in editor memory until model_close; nothing is written to disk until model_save.";
}

Ref< Json > ModelOpenTool::getInputSchema() const
{
	auto str = [](const wchar_t* d) {
		Ref< Json > p = Json::createObject();
		p->setString(L"type", L"string");
		p->setString(L"description", d);
		return p;
	};

	Ref< Json > properties = Json::createObject();
	properties->set(L"file", str(L"Model file path to read via ModelFormat (absolute or relative to Pipeline.AssetPath)."));
	properties->set(L"filter", str(L"Optional import filter passed to ModelFormat (e.g. a node/mesh name)."));
	properties->set(L"meshAsset", str(L"MeshAsset instance guid or database path; its referenced source model file is read."));

	Ref< Json > blank = Json::createObject();
	blank->setString(L"type", L"boolean");
	blank->setString(L"description", L"Create an empty model from scratch (no source file).");
	properties->set(L"blank", blank);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > ModelOpenTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< const PropertyGroup > settings = m_editor->getSettings();
	const std::wstring assetPath = settings ? settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"") : L"";

	const std::wstring fileStr = (arguments && arguments->getMember(L"file")) ? arguments->getMember(L"file")->getString() : L"";
	const std::wstring meshAssetRef = (arguments && arguments->getMember(L"meshAsset")) ? arguments->getMember(L"meshAsset")->getString() : L"";
	const bool blank = arguments && arguments->getMember(L"blank") && arguments->getMember(L"blank")->getBoolean();

	Ref< model::Model > model;
	ModelSession::SourceKind sourceKind = ModelSession::SourceKind::Blank;
	std::wstring resolvedFile;
	Guid meshAssetGuid;
	std::wstring importFilter;

	if (!fileStr.empty())
	{
		const std::wstring filter = (arguments->getMember(L"filter")) ? arguments->getMember(L"filter")->getString() : L"";
		const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath), Path(fileStr));
		model = model::ModelFormat::readAny(filePath, filter);
		if (!model)
		{
			outError = L"Failed to read model from \"" + filePath.getPathName() + L"\" (unsupported format or file not found).";
			return nullptr;
		}
		sourceKind = ModelSession::SourceKind::File;
		resolvedFile = filePath.getPathName();
		importFilter = filter;
	}
	else if (!meshAssetRef.empty())
	{
		Ref< db::Database > database = m_editor->getSourceDatabase();
		if (!database)
		{
			outError = L"No source database is currently open.";
			return nullptr;
		}
		Ref< db::Instance > instance;
		const Guid g(meshAssetRef);
		if (g.isValid())
			instance = database->getInstance(g);
		if (!instance)
			instance = database->getInstance(meshAssetRef);
		if (!instance)
		{
			outError = L"No instance found for meshAsset \"" + meshAssetRef + L"\".";
			return nullptr;
		}

		Ref< ISerializable > object;
		mesh::MeshAsset* meshAsset = loadMeshAsset(instance, object, outError);
		if (!meshAsset)
			return nullptr;

		importFilter = meshAsset->getImportFilter();
		const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath), meshAsset->getFileName());
		model = model::ModelFormat::readAny(filePath, importFilter);
		if (!model)
		{
			outError = L"Failed to read the MeshAsset's source model file \"" + filePath.getPathName() + L"\".";
			return nullptr;
		}
		sourceKind = ModelSession::SourceKind::MeshAsset;
		resolvedFile = filePath.getPathName();
		meshAssetGuid = instance->getGuid();
	}
	else if (blank)
	{
		model = new model::Model();
		sourceKind = ModelSession::SourceKind::Blank;
	}
	else
	{
		outError = L"Provide one of \"file\", \"meshAsset\", or \"blank\":true.";
		return nullptr;
	}

	const int32_t handle = m_session->open(model, sourceKind, resolvedFile, meshAssetGuid, importFilter);

	Ref< Json > source = Json::createObject();
	switch (sourceKind)
	{
	case ModelSession::SourceKind::File:
		source->setString(L"kind", L"file");
		source->setString(L"file", resolvedFile);
		break;
	case ModelSession::SourceKind::MeshAsset:
		source->setString(L"kind", L"meshAsset");
		source->setString(L"file", resolvedFile);
		source->setString(L"meshAsset", meshAssetGuid.format());
		break;
	default:
		source->setString(L"kind", L"blank");
		break;
	}

	Ref< Json > result = Json::createObject();
	result->set(L"handle", Json::createNumber((int64_t)handle));
	result->set(L"source", source);
	result->set(L"summary", modelCountsToJson(model));
	return result;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelListTool.h"

#include "MCP/Server/Editor/ModelSession.h"
#include "MCP/Server/Json.h"
#include "Model/Model.h"

namespace traktor::mcp
{
namespace
{

const wchar_t* sourceKindName(ModelSession::SourceKind kind)
{
	switch (kind)
	{
	case ModelSession::SourceKind::File:
		return L"file";
	case ModelSession::SourceKind::MeshAsset:
		return L"meshAsset";
	default:
		return L"blank";
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelListTool", ModelListTool, IMcpTool)

ModelListTool::ModelListTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelListTool::getName() const
{
	return L"model_list";
}

std::wstring ModelListTool::getDescription() const
{
	return L"List the models currently open in the model session, with their handle, origin (blank/file/meshAsset) and a compact element-count summary. Use this to recover handles opened earlier in the session.";
}

Ref< Json > ModelListTool::getInputSchema() const
{
	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", Json::createObject());
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > ModelListTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< Json > models = Json::createArray();
	for (const int32_t handle : m_session->handles())
	{
		const ModelSession::Entry* entry = m_session->entry(handle);
		if (!entry || !entry->model)
			continue;
		model::Model* model = entry->model;

		Ref< Json > counts = Json::createObject();
		counts->set(L"materialCount", Json::createNumber((int64_t)model->getMaterialCount()));
		counts->set(L"vertexCount", Json::createNumber((int64_t)model->getVertexCount()));
		counts->set(L"polygonCount", Json::createNumber((int64_t)model->getPolygonCount()));
		counts->set(L"positionCount", Json::createNumber((int64_t)model->getPositionCount()));
		counts->set(L"jointCount", Json::createNumber((int64_t)model->getJointCount()));
		counts->set(L"animationCount", Json::createNumber((int64_t)model->getAnimationCount()));

		Ref< Json > entryJson = Json::createObject();
		entryJson->set(L"handle", Json::createNumber((int64_t)handle));
		entryJson->setString(L"source", sourceKindName(entry->sourceKind));
		if (!entry->filePath.empty())
			entryJson->setString(L"file", entry->filePath);
		if (entry->meshAssetGuid.isNotNull())
			entryJson->setString(L"meshAsset", entry->meshAssetGuid.format());
		entryJson->set(L"summary", counts);
		models->push(entryJson);
	}

	Ref< Json > result = Json::createObject();
	result->set(L"models", models);
	result->set(L"count", Json::createNumber((int64_t)models->size()));
	return result;
}

}

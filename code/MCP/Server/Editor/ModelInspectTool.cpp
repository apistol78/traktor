/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelInspectTool.h"

#include "MCP/Server/Editor/ModelSession.h"
#include "MCP/Server/Editor/ModelToolSupport.h"
#include "MCP/Server/Json.h"
#include "Model/Model.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelInspectTool", ModelInspectTool, IMcpTool)

ModelInspectTool::ModelInspectTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelInspectTool::getName() const
{
	return L"model_inspect";
}

std::wstring ModelInspectTool::getDescription() const
{
	return L"Summarize an open model (by \"handle\"): element counts, bounding box, and the full material, joint (skeleton), animation, texcoord-channel and blend-target lists. Does NOT dump raw geometry (positions/vertices/polygons) - use model_get_elements to page through those - so the payload stays bounded regardless of mesh size.";
}

Ref< Json > ModelInspectTool::getInputSchema() const
{
	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of an open model (from model_open).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"handle"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ModelInspectTool::invoke(const Json* arguments, std::wstring& outError)
{
	int32_t handle = 0;
	model::Model* model = resolveModel(m_session, arguments, handle, outError);
	if (!model)
		return nullptr;

	Ref< Json > result = modelInspectToJson(model);
	result->set(L"handle", Json::createNumber((int64_t)handle));
	return result;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelCloseTool.h"

#include "MCP/Server/Editor/ModelSession.h"
#include "MCP/Server/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelCloseTool", ModelCloseTool, IMcpTool)

ModelCloseTool::ModelCloseTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelCloseTool::getName() const
{
	return L"model_close";
}

std::wstring ModelCloseTool::getDescription() const
{
	return L"Release a working model from the model session so its memory is freed. Pass \"handle\" to close one model, or \"all\":true to close every open model. This does not save; call model_save first to persist unsaved edits.";
}

Ref< Json > ModelCloseTool::getInputSchema() const
{
	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of the model to close.");

	Ref< Json > all = Json::createObject();
	all->setString(L"type", L"boolean");
	all->setString(L"description", L"Close every open model.");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);
	properties->set(L"all", all);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > ModelCloseTool::invoke(const Json* arguments, std::wstring& outError)
{
	if (arguments && arguments->getMember(L"all") && arguments->getMember(L"all")->getBoolean())
	{
		const int32_t closed = m_session->closeAll();
		Ref< Json > result = Json::createObject();
		result->set(L"closed", Json::createNumber((int64_t)closed));
		return result;
	}

	const Json* h = arguments ? arguments->getMember(L"handle") : nullptr;
	if (!h || !h->isNumber())
	{
		outError = L"Provide \"handle\" (integer) or \"all\":true.";
		return nullptr;
	}
	const int32_t handle = (int32_t)h->getNumber();
	if (!m_session->close(handle))
	{
		outError = L"No open model with handle " + std::to_wstring(handle) + L".";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->set(L"closed", Json::createNumber((int64_t)1));
	result->set(L"handle", Json::createNumber((int64_t)handle));
	return result;
}

}

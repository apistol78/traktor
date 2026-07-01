/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/IsEditorOpenTool.h"

#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Json.h"

#include <memory>

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.IsEditorOpenTool", IsEditorOpenTool, IMcpTool)

IsEditorOpenTool::IsEditorOpenTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring IsEditorOpenTool::getName() const
{
	return L"is_editor_open";
}

std::wstring IsEditorOpenTool::getDescription() const
{
	return L"Check whether a database instance is currently open in an editor. Identify the instance by \"guid\" (preferred) or \"path\". Returns \"open\"=true if a document for the instance is open in any tab.";
}

Ref< Json > IsEditorOpenTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the instance (alternative to guid).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > IsEditorOpenTool::invoke(const Json* arguments, std::wstring& outError)
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
			outError = L"No instance found for the given guid/path.";
		return nullptr;
	}

	// Reading the open documents touches UI state, so run it on the UI thread.
	auto open = std::make_shared< bool >(false);
	if (!runOnUiThread([this, instance, open]() {
			*open = m_editor->isEditorOpen(instance);
		}))
	{
		outError = L"Timed out waiting for the editor UI thread.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setBoolean(L"open", *open);
	return result;
}

}

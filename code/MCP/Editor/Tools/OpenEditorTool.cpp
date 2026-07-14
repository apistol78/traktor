/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/OpenEditorTool.h"

#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/McpToolSupport.h"
#include "MCP/Editor/Json.h"

#include <memory>

namespace traktor::mcp
{
namespace
{

struct OpenResult
{
	bool alreadyOpen = false;
	bool opened = false;
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.OpenEditorTool", OpenEditorTool, IMcpTool)

OpenEditorTool::OpenEditorTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring OpenEditorTool::getName() const
{
	return L"open_editor";
}

std::wstring OpenEditorTool::getDescription() const
{
	return L"Open a database instance in the editor (as an editor tab/page). Identify the instance by \"guid\" (preferred) or \"path\". If an editor for the instance is already open its tab is activated instead. \"alreadyOpen\" tells whether an editor was already open; \"opened\"=true if it is now open. Pairs with is_editor_open and close_editor.";
}

Ref< Json > OpenEditorTool::getInputSchema() const
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

Ref< Json > OpenEditorTool::invoke(const Json* arguments, std::wstring& outError)
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

	// Opening an editor manipulates UI widgets and opens a database transaction,
	// both bound to the main thread; marshal the work there. openEditor itself
	// re-activates an already-open tab, but capturing the open state up front lets
	// the result report whether this call actually opened a new editor.
	auto r = std::make_shared< OpenResult >();
	if (!runOnUiThread([this, instance, r]() {
			r->alreadyOpen = m_editor->isEditorOpen(instance);
			r->opened = m_editor->openEditor(instance);
		}))
	{
		outError = L"Timed out waiting for the editor UI thread.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setBoolean(L"alreadyOpen", r->alreadyOpen);
	result->setBoolean(L"opened", r->opened);
	return result;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/CloseEditorTool.h"

#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Json.h"

#include <memory>

namespace traktor::mcp
{
namespace
{

struct CloseResult
{
	bool wasOpen = false;
	bool closed = false;
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.CloseEditorTool", CloseEditorTool, IMcpTool)

CloseEditorTool::CloseEditorTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring CloseEditorTool::getName() const
{
	return L"close_editor";
}

std::wstring CloseEditorTool::getDescription() const
{
	return L"Close the editor for a database instance. Identify the instance by \"guid\" (preferred) or \"path\". An editor with unsaved changes is left open and \"closed\"=false is returned; pass \"forceCloseIfUnsaved\"=true to discard those changes and close anyway. \"wasOpen\" tells whether an editor was open at all.";
}

Ref< Json > CloseEditorTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the instance (alternative to guid).");

	Ref< Json > forceProperty = Json::createObject();
	forceProperty->setString(L"type", L"boolean");
	forceProperty->setString(L"description", L"Close even when the editor has unsaved changes, discarding them (default false).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"forceCloseIfUnsaved", forceProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > CloseEditorTool::invoke(const Json* arguments, std::wstring& outError)
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

	const bool force = arguments && arguments->getMember(L"forceCloseIfUnsaved") && arguments->getMember(L"forceCloseIfUnsaved")->getBoolean();

	// Closing an editor manipulates UI widgets, so marshal the work onto the main
	// thread. Capture the open state up front so the result can distinguish "was
	// not open" from "open but left alone because of unsaved changes".
	auto r = std::make_shared< CloseResult >();
	if (!runOnUiThread([this, instance, force, r]() {
			r->wasOpen = m_editor->isEditorOpen(instance);
			r->closed = r->wasOpen ? m_editor->closeEditor(instance, force) : false;
		}))
	{
		outError = L"Timed out waiting for the editor UI thread.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setBoolean(L"wasOpen", r->wasOpen);
	result->setBoolean(L"closed", r->closed);
	return result;
}

}

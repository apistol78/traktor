/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/BuildAssetTool.h"

#include "Core/Guid.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.BuildAssetTool", BuildAssetTool, IMcpTool)

BuildAssetTool::BuildAssetTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring BuildAssetTool::getName() const
{
	return L"build_asset";
}

std::wstring BuildAssetTool::getDescription() const
{
	return L"Build (compile) a source asset through the editor pipeline into the output database so it becomes usable at runtime. Identify the asset by \"guid\" (preferred) or \"path\". Pass \"rebuild\"=true to force a full rebuild ignoring caches (default false), and \"wait\"=false to return immediately instead of blocking until the build finishes (default true).";
}

Ref< Json > BuildAssetTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the asset instance to build (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the asset instance (alternative to guid).");

	Ref< Json > rebuildProperty = Json::createObject();
	rebuildProperty->setString(L"type", L"boolean");
	rebuildProperty->setString(L"description", L"Force a full rebuild ignoring caches (default false).");

	Ref< Json > waitProperty = Json::createObject();
	waitProperty->setString(L"type", L"boolean");
	waitProperty->setString(L"description", L"Block until the build has finished (default true).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"rebuild", rebuildProperty);
	properties->set(L"wait", waitProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > BuildAssetTool::invoke(const Json* arguments, std::wstring& outError)
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

	const Guid guid = instance->getGuid();
	const bool rebuild = arguments && arguments->getMember(L"rebuild") && arguments->getMember(L"rebuild")->getBoolean();
	const bool wait = !arguments || !arguments->getMember(L"wait") || arguments->getMember(L"wait")->getBoolean();

	m_editor->buildAsset(guid, rebuild);
	if (wait)
		m_editor->buildWaitUntilFinished();

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", guid.format());
	result->setString(L"path", instance->getPath());
	result->setBoolean(L"rebuild", rebuild);
	result->setBoolean(L"waited", wait);
	result->setBoolean(L"built", true);
	return result;
}

}

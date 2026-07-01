/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/GetWorkspaceInfoTool.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.GetWorkspaceInfoTool", GetWorkspaceInfoTool, IMcpTool)

GetWorkspaceInfoTool::GetWorkspaceInfoTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring GetWorkspaceInfoTool::getName() const
{
	return L"get_workspace_info";
}

std::wstring GetWorkspaceInfoTool::getDescription() const
{
	return L"Report the editor workspace configuration: the source-asset path (Pipeline.AssetPath, both as configured and resolved to an absolute filesystem path) and the source/output database connection strings. Use this to locate where source assets (.fbx, .blend, textures, ...) referenced by asset file names live on disk.";
}

Ref< Json > GetWorkspaceInfoTool::getInputSchema() const
{
	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", Json::createObject());
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > GetWorkspaceInfoTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< const PropertyGroup > settings = m_editor->getSettings();
	if (!settings)
	{
		outError = L"Editor settings are not available.";
		return nullptr;
	}

	const std::wstring assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	Ref< Json > result = Json::createObject();
	result->setString(L"assetPath", assetPath);
	if (!assetPath.empty())
	{
		// Resolve relative to the editor's current working directory (the workspace root).
		const Path absolutePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath));
		result->setString(L"assetPathAbsolute", absolutePath.getPathName());
	}
	result->setString(L"sourceDatabase", settings->getProperty< std::wstring >(L"Editor.SourceDatabase", L""));
	result->setString(L"outputDatabase", settings->getProperty< std::wstring >(L"Editor.OutputDatabase", L""));
	result->setString(L"targetTitle", settings->getProperty< std::wstring >(L"Editor.TargetTitle", L""));
	return result;
}

}

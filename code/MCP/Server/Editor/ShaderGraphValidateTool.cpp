/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ShaderGraphValidateTool.h"

#include "Core/Guid.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/ShaderGraphToolSupport.h"
#include "MCP/Server/Json.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderGraphValidateTool", ShaderGraphValidateTool, IMcpTool)

ShaderGraphValidateTool::ShaderGraphValidateTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring ShaderGraphValidateTool::getName() const
{
	return L"validate_shader_graph";
}

std::wstring ShaderGraphValidateTool::getDescription() const
{
	return L"Resolve a shader graph's fragments and validate it. Returns whether the graph is valid, its estimated type (program/fragment) and a list of offending nodes. Use this as a feedback loop when authoring shader graphs.";
}

Ref< Json > ShaderGraphValidateTool::getInputSchema() const
{
	return targetSchema(false);
}

Ref< Json > ShaderGraphValidateTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	Guid guid;
	std::wstring name;
	Ref< render::ShaderGraph > shaderGraph = loadShaderGraph(database, arguments, guid, name, outError);
	if (!shaderGraph)
		return nullptr;

	bool valid = false;
	Ref< Json > result = validateGraph(database, shaderGraph, guid, valid);
	result->setString(L"guid", guid.format());
	return result;
}

}

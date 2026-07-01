/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ShaderGraphGetTool.h"

#include "Core/Guid.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/ShaderGraphToolSupport.h"
#include "MCP/Server/Json.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTypePropagation.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderGraphGetTool", ShaderGraphGetTool, IMcpTool)

ShaderGraphGetTool::ShaderGraphGetTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring ShaderGraphGetTool::getName() const
{
	return L"get_shader_graph";
}

std::wstring ShaderGraphGetTool::getDescription() const
{
	return L"Read a shader graph as a node/edge graph. Nodes have stable local ids (n0, n1, ...), a type, info, pins and position; edges connect node-id + pin-name pairs. External nodes carry the referenced fragment guid and name. Pass resolve=true to inline all referenced fragments.";
}

Ref< Json > ShaderGraphGetTool::getInputSchema() const
{
	return targetSchema(true);
}

Ref< Json > ShaderGraphGetTool::invoke(const Json* arguments, std::wstring& outError)
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

	const bool resolve = arguments && arguments->getMember(L"resolve") && arguments->getMember(L"resolve")->getBoolean();
	if (resolve)
	{
		Ref< render::ShaderGraph > resolved = resolveShaderGraph(database, shaderGraph, guid);
		if (!resolved)
		{
			outError = L"Failed to resolve shader graph fragments (a referenced fragment may be missing).";
			return nullptr;
		}
		shaderGraph = resolved;
	}

	// Propagate pin types so each pin can be annotated with its resolved type.
	// Propagation requires traits for every node, so it is only valid on a
	// fully resolved graph (External nodes have no trait).
	const render::ShaderGraphTypePropagation typePropagation(shaderGraph, guid);

	Ref< Json > result = buildGraphIr(shaderGraph, database, &typePropagation);
	result->setString(L"guid", guid.format());
	result->setString(L"name", name);
	result->setString(L"graphType", render::ShaderGraphValidator(shaderGraph).estimateType() == render::ShaderGraphValidator::SgtFragment ? L"fragment" : L"program");
	result->setBoolean(L"resolved", resolve);
	result->setBoolean(L"typesResolved", typePropagation.valid());
	return result;
}

}

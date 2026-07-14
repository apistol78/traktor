/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ShaderGraphUpdateTool.h"

#include "Core/Guid.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/McpToolSupport.h"
#include "MCP/Editor/ShaderGraphToolSupport.h"
#include "MCP/Editor/Json.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderGraphUpdateTool", ShaderGraphUpdateTool, IMcpTool)

ShaderGraphUpdateTool::ShaderGraphUpdateTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring ShaderGraphUpdateTool::getName() const
{
	return L"update_shader_graph";
}

std::wstring ShaderGraphUpdateTool::getDescription() const
{
	return L"Replace the contents of an existing shader graph instance (identified by guid or path) with a node/edge IR. The graph is validated and only committed when valid, unless force=true.";
}

Ref< Json > ShaderGraphUpdateTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the shader graph instance to update (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the shader graph instance (alternative to guid).");

	Ref< Json > graphProperty = Json::createObject();
	graphProperty->setString(L"type", L"object");
	graphProperty->setString(L"description", L"The shader graph IR: { \"nodes\": [...], \"edges\": [...] }.");

	Ref< Json > forceProperty = Json::createObject();
	forceProperty->setString(L"type", L"boolean");
	forceProperty->setString(L"description", L"Commit even if validation fails (default false).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"graph", graphProperty);
	properties->set(L"force", forceProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"graph"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ShaderGraphUpdateTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	// Recover any structured arguments a client delivered as JSON-encoded strings.
	Ref< Json > coerced = coerceStructuredArguments(arguments);
	arguments = coerced;

	Ref< db::Instance > instance = resolveInstance(database, arguments, outError);
	if (!instance)
	{
		if (outError.empty())
			outError = L"No shader graph instance found for the given guid/path.";
		return nullptr;
	}
	if (!instance->getObject< render::ShaderGraph >())
	{
		outError = L"Target instance is not a ShaderGraph (primary type: " + instance->getPrimaryTypeName() + L").";
		return nullptr;
	}

	const Guid guid = instance->getGuid();

	Ref< Json > warnings = Json::createArray();
	std::wstring error;
	Ref< render::ShaderGraph > graph = graphFromIr(database, arguments->getMember(L"graph"), error, warnings);
	if (!graph)
	{
		outError = error;
		return nullptr;
	}

	bool valid = false;
	Ref< Json > validation = validateGraph(database, graph, guid, valid);
	const bool force = arguments->getMember(L"force") && arguments->getMember(L"force")->getBoolean();

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", guid.format());
	result->setString(L"path", instance->getPath());
	result->set(L"validation", validation);
	result->set(L"warnings", warnings);

	if (!valid && !force)
	{
		result->setBoolean(L"committed", false);
		result->setString(L"message", L"Graph not committed because validation failed; fix the reported errors or pass force=true.");
		return result;
	}

	if (!instance->checkout())
	{
		outError = L"Failed to checkout instance for writing.";
		return nullptr;
	}
	if (!instance->setObject(graph) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the updated shader graph.";
		return nullptr;
	}

	result->setBoolean(L"committed", true);
	return result;
}

}

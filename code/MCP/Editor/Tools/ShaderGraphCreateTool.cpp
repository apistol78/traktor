/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ShaderGraphCreateTool.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderGraphCreateTool", ShaderGraphCreateTool, IMcpTool)

ShaderGraphCreateTool::ShaderGraphCreateTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring ShaderGraphCreateTool::getName() const
{
	return L"create_shader_graph";
}

std::wstring ShaderGraphCreateTool::getDescription() const
{
	return L"Create a new shader graph instance from a node/edge IR (the same shape get_shader_graph returns). Node 'type' is the short node type name (see list_shader_node_types); 'properties' set scalar/string/guid node values; edges connect node-id + pin-name pairs. The graph is validated and only committed when valid, unless force=true.";
}

Ref< Json > ShaderGraphCreateTool::getInputSchema() const
{
	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path to create the shader graph at (e.g. \"Shaders/MyShader\").");

	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Optional explicit guid for the new instance (otherwise one is generated).");

	Ref< Json > graphProperty = Json::createObject();
	graphProperty->setString(L"type", L"object");
	graphProperty->setString(L"description", L"The shader graph IR: { \"nodes\": [...], \"edges\": [...] }.");

	Ref< Json > forceProperty = Json::createObject();
	forceProperty->setString(L"type", L"boolean");
	forceProperty->setString(L"description", L"Commit even if validation fails (default false).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"path", pathProperty);
	properties->set(L"guid", guidProperty);
	properties->set(L"graph", graphProperty);
	properties->set(L"force", forceProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"path"));
	required->push(Json::createString(L"graph"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ShaderGraphCreateTool::invoke(const Json* arguments, std::wstring& outError)
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

	const std::wstring path = (arguments && arguments->getMember(L"path")) ? arguments->getMember(L"path")->getString() : L"";
	if (path.empty())
	{
		outError = L"Missing \"path\".";
		return nullptr;
	}
	if (database->getInstance(path))
	{
		outError = L"An instance already exists at \"" + path + L"\"; use update_shader_graph to replace it.";
		return nullptr;
	}

	Ref< Json > warnings = Json::createArray();
	std::wstring error;
	Ref< render::ShaderGraph > graph = graphFromIr(database, arguments->getMember(L"graph"), error, warnings);
	if (!graph)
	{
		outError = error;
		return nullptr;
	}

	Guid guid;
	if (arguments->getMember(L"guid"))
	{
		const Guid g(arguments->getMember(L"guid")->getString());
		if (g.isValid())
			guid = g;
	}
	if (!guid.isValid())
		guid = Guid::create();

	bool valid = false;
	Ref< Json > validation = validateGraph(database, graph, guid, valid);
	const bool force = arguments->getMember(L"force") && arguments->getMember(L"force")->getBoolean();

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", guid.format());
	result->setString(L"path", path);
	result->set(L"validation", validation);
	result->set(L"warnings", warnings);

	if (!valid && !force)
	{
		result->setBoolean(L"committed", false);
		result->setString(L"message", L"Graph not committed because validation failed; fix the reported errors or pass force=true.");
		return result;
	}

	Ref< db::Instance > instance = database->createInstance(path, db::CifReplaceExisting, &guid);
	if (!instance)
	{
		outError = L"Failed to create instance at \"" + path + L"\".";
		return nullptr;
	}
	if (!instance->setObject(graph) || !instance->commit())
	{
		outError = L"Failed to commit the new shader graph.";
		return nullptr;
	}

	result->setBoolean(L"committed", true);
	return result;
}

}

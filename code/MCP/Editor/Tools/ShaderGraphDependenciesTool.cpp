/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ShaderGraphDependenciesTool.h"

#include "Core/Guid.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/ShaderGraphToolSupport.h"
#include "MCP/Editor/Json.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/ShaderGraph.h"

#include <functional>
#include <set>

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderGraphDependenciesTool", ShaderGraphDependenciesTool, IMcpTool)

ShaderGraphDependenciesTool::ShaderGraphDependenciesTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring ShaderGraphDependenciesTool::getName() const
{
	return L"get_shader_dependencies";
}

std::wstring ShaderGraphDependenciesTool::getDescription() const
{
	return L"List the shader fragments a shader graph depends on, discovered through its External nodes. By default the traversal is recursive (fragments of fragments).";
}

Ref< Json > ShaderGraphDependenciesTool::getInputSchema() const
{
	Ref< Json > schema = targetSchema(false);

	Ref< Json > recursiveProperty = Json::createObject();
	recursiveProperty->setString(L"type", L"boolean");
	recursiveProperty->setString(L"description", L"Recurse into referenced fragments (default true).");
	schema->getMember(L"properties")->set(L"recursive", recursiveProperty);
	return schema;
}

Ref< Json > ShaderGraphDependenciesTool::invoke(const Json* arguments, std::wstring& outError)
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

	const bool recursive = !(arguments && arguments->getMember(L"recursive") && !arguments->getMember(L"recursive")->getBoolean(true));

	Ref< Json > list = Json::createArray();
	std::set< std::wstring > seen;

	std::function< void(const render::ShaderGraph*, bool) > walk = [&](const render::ShaderGraph* graph, bool direct) {
		for (auto external : graph->findNodesOf< render::External >())
		{
			const Guid& fragmentGuid = external->getFragmentGuid();
			if (!fragmentGuid.isValid())
				continue;

			const std::wstring key = fragmentGuid.format();
			if (!seen.insert(key).second)
				continue;

			Ref< db::Instance > instance = database->getInstance(fragmentGuid);

			Ref< Json > entry = Json::createObject();
			entry->setString(L"guid", key);
			entry->setString(L"name", instance ? instance->getName() : L"(missing)");
			if (instance)
				entry->setString(L"path", instance->getPath());
			entry->setBoolean(L"direct", direct);
			entry->setBoolean(L"resolved", instance != nullptr);
			list->push(entry);

			if (recursive)
			{
				Ref< render::ShaderGraph > childGraph = database->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
				if (childGraph)
					walk(childGraph, false);
			}
		}
	};

	walk(shaderGraph, true);

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", guid.format());
	result->set(L"dependencies", list);
	result->setNumber(L"count", (int64_t)list->size());
	return result;
}

}

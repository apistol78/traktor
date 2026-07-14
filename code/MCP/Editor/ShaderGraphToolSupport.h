/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "MCP/Editor/McpToolSupport.h"

#include <string>

namespace traktor
{

class Guid;

}

namespace traktor::db
{

class Database;
class Instance;

}

namespace traktor::render
{

class Node;
class ShaderGraph;
class ShaderGraphTypePropagation;

}

namespace traktor::mcp
{

class Json;

/*! \name Shared helpers for the shader graph tools.
 *
 * These convert between Traktor shader graphs and the curated node/edge JSON IR
 * the tools exchange with MCP clients, and load/resolve/validate graphs against
 * the editor database.
 * \{
 */

/*! Strip the namespace from a fully qualified RTTI type name. */
std::wstring shortTypeName(const std::wstring& fullName);

/*! True if the node type (short name, e.g. L"Uniform") is tagged T_DEPRECATED. */
bool isDeprecatedNode(const std::wstring& shortType);

/*! Load (and parameter/uniform-link) the targeted shader graph instance.
 *
 * \param arguments Tool arguments carrying a "guid" or "path".
 */
Ref< render::ShaderGraph > loadShaderGraph(db::Database* database, const Json* arguments, Guid& outGuid, std::wstring& outName, std::wstring& outError);

/*! Resolve External fragments (and parameter/uniform declarations) against the database. */
Ref< render::ShaderGraph > resolveShaderGraph(db::Database* database, const render::ShaderGraph* shaderGraph, const Guid& guid);

/*! Build the curated node/edge IR for a shader graph. */
Ref< Json > buildGraphIr(const render::ShaderGraph* shaderGraph, db::Database* database, const render::ShaderGraphTypePropagation* typePropagation = nullptr);

/*! JSON Schema for a tool that targets a single shader graph instance (guid/path[/resolve]). */
Ref< Json > targetSchema(bool includeResolve);

/*! Build a shader graph from the node/edge IR. */
Ref< render::ShaderGraph > graphFromIr(db::Database* database, const Json* graphJson, std::wstring& outError, Json* warnings);

/*! Resolve fragments and validate a graph, describing the result as JSON. */
Ref< Json > validateGraph(db::Database* database, const render::ShaderGraph* shaderGraph, const Guid& guid, bool& outValid);

/*! \} */

}

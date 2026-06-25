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

#include <string>

namespace traktor::db
{

class Database;
class Instance;

}

namespace traktor::mcp
{

class Json;

/*! Resolve the targeted instance (guid or path) from a tool's arguments.
 * \ingroup MCP
 *
 * Common to every tool that operates on a single database instance: reads a
 * "guid" (preferred) or "path" member from the arguments. Returns null and sets
 * \a outError on a malformed guid; returns null without an error when simply not
 * found, so callers can supply their own "not found" message.
 */
Ref< db::Instance > resolveInstance(db::Database* database, const Json* arguments, std::wstring& outError);

/*! Coerce JSON-encoded string arguments back into structured JSON.
 * \ingroup MCP
 *
 * Some MCP clients deliver array/object tool arguments as a JSON-encoded string
 * (a parameter with no single concrete JSON type tends to be stringified).
 * Returns a shallow copy of \a arguments in which every top-level member that is
 * such a string - one that decodes to a JSON array or object - is replaced by
 * the decoded value; all other members (and non-parseable strings) are shared
 * unchanged. Returns \a arguments wrapped unchanged when it is not an object.
 *
 * Call this once at the top of invoke() in any tool that reads array/object
 * parameters, then read members from the returned object as usual.
 */
Ref< Json > coerceStructuredArguments(const Json* arguments);

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"

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

}

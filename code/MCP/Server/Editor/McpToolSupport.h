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

#include <functional>
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

/*! Run a function on the editor's UI (main) thread and block until it completes.
 * \ingroup MCP
 *
 * MCP tools run on the server's worker thread, but editor operations that touch
 * UI widgets or open database transactions (opening/closing editors, inspecting
 * open documents) are bound to the main thread - calling them from the worker
 * thread trips a fatal transaction-thread assertion. This marshals \a fn onto the
 * UI thread via ui::Application::defer() and waits for it to run, so a tool can
 * perform such work and still return its result synchronously.
 *
 * Returns true when \a fn ran, false if it did not complete within \a timeoutMs
 * (e.g. the UI thread is blocked) or there is no running application. The work is
 * held alive independently of this call, so a timeout never leaves \a fn dangling.
 */
bool runOnUiThread(const std::function< void() >& fn, int32_t timeoutMs = 8000);

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

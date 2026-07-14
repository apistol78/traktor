/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MCP_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mcp
{

class Json;

/*! MCP tool interface.
 * \ingroup MCP
 *
 * A tool is a named, callable capability exposed to an MCP client through
 * the "tools/list" and "tools/call" methods. Concrete tools are registered
 * with an \a McpServer.
 */
class T_DLLCLASS IMcpTool : public Object
{
	T_RTTI_CLASS;

public:
	/*! Unique tool name as referenced by "tools/call". */
	virtual std::wstring getName() const = 0;

	/*! Human readable description of what the tool does. */
	virtual std::wstring getDescription() const = 0;

	/*! JSON Schema object describing the tool's "arguments". */
	virtual Ref< Json > getInputSchema() const = 0;

	/*! Invoke the tool.
	 *
	 * \param arguments Argument object as supplied by the client; may be null.
	 * \param outError Set to a non-empty message if invocation failed.
	 * \return Result value; serialized into the tool result content. Null is
	 *         permitted (treated as an empty result) when \a outError is empty.
	 */
	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) = 0;
};

}

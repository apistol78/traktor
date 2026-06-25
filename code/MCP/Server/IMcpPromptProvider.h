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
#if defined(T_MCP_SERVER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mcp
{

class Json;

/*! MCP prompt provider interface.
 * \ingroup MCP
 *
 * Supplies the prompts exposed through the "prompts/list" and "prompts/get"
 * methods. The server core is database-agnostic; concrete providers (registered
 * with an \a McpServer) bridge to a domain source such as the editor database.
 */
class T_DLLCLASS IMcpPromptProvider : public Object
{
	T_RTTI_CLASS;

public:
	/*! Append this provider's prompt descriptors to \a outPrompts (a JSON array).
	 *
	 * Each descriptor is an object: { "name", "description", "arguments": [ { "name", "description", "required" } ] }.
	 */
	virtual void listPrompts(Json* outPrompts) = 0;

	/*! Resolve a named prompt into a "prompts/get" result.
	 *
	 * \param name Requested prompt name.
	 * \param arguments Supplied argument object (may be null).
	 * \param outError Set on failure.
	 * \return The result object ({ "description", "messages": [...] }) when this
	 *         provider owns the name; null with an empty \a outError when it does
	 *         not (so other providers may try); null with a non-empty \a outError
	 *         on failure.
	 */
	virtual Ref< Json > getPrompt(const std::wstring& name, const Json* arguments, std::wstring& outError) = 0;
};

}

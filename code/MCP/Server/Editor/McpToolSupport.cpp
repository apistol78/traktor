/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Guid.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/McpToolSupport.h"

namespace traktor::mcp
{

Ref< db::Instance > resolveInstance(db::Database* database, const Json* arguments, std::wstring& outError)
{
	if (!arguments)
	{
		outError = L"Missing arguments; expected \"guid\" or \"path\".";
		return nullptr;
	}

	const std::wstring guidText = arguments->getMember(L"guid") ? arguments->getMember(L"guid")->getString() : L"";
	if (!guidText.empty())
	{
		const Guid guid(guidText);
		if (!guid.isValid())
		{
			outError = L"Invalid guid: " + guidText;
			return nullptr;
		}
		return database->getInstance(guid);
	}

	if (arguments->getMember(L"path"))
		return database->getInstance(arguments->getMember(L"path")->getString());

	outError = L"Missing \"guid\" or \"path\".";
	return nullptr;
}

}

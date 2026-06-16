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
#include "MCP/Server/IMcpTool.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mcp
{

class Json;

/*! Tool: report the editor workspace configuration.
 * \ingroup MCP
 *
 * Surfaces editor settings that are not visible through the database-oriented
 * tools: the source-asset path (Pipeline.AssetPath, both as configured and
 * resolved to an absolute filesystem path) and the source/output database
 * connection strings.
 */
class GetWorkspaceInfoTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit GetWorkspaceInfoTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

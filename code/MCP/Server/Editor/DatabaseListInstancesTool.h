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

/*! Tool: list instances in the editor's source database.
 * \ingroup MCP
 *
 * Returns the name, guid, primary type and path of each asset instance, with
 * optional filtering by primary type name and group path.
 */
class DatabaseListInstancesTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit DatabaseListInstancesTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

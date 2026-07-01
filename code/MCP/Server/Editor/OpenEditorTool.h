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
#include "MCP/Server/IMcpTool.h"

#include <string>

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mcp
{

class Json;

/*! Tool: open an instance in the editor.
 * \ingroup MCP
 *
 * Identifies the instance by "guid" (preferred) or "path" and opens it as an
 * editor tab/page using the editor registered for its type, activating the
 * existing tab if it is already open. Counterpart to IsEditorOpenTool and
 * CloseEditorTool, which track these same tab documents.
 */
class OpenEditorTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit OpenEditorTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

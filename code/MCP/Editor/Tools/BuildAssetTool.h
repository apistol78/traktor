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
#include "MCP/Editor/IMcpTool.h"

#include <string>

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mcp
{

class Json;

/*! Tool: build a source asset through the editor pipeline.
 * \ingroup MCP
 *
 * Compiles a source instance (identified by guid or path) into the output
 * database so it becomes usable at runtime. Optionally forces a full rebuild
 * and/or blocks until the build has finished.
 */
class BuildAssetTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit BuildAssetTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

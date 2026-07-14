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
class ModelSession;

/*! Tool: open a working model into the model session.
 * \ingroup MCP
 *
 * Loads a model from a file (via ModelFormat), from a MeshAsset's referenced
 * source file, or creates a blank model from scratch, and returns a handle used
 * by the other model_* tools.
 */
class ModelOpenTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit ModelOpenTool(editor::IEditor* editor, ModelSession* session);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
	ModelSession* m_session;
};

}

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

/*! Tool: query world heights from a heightfield asset.
 * \ingroup MCP
 *
 * Reads a traktor.hf.HeightfieldAsset and samples the terrain at a set of world
 * XZ coordinates, returning the interpolated height (Y), the cut flag and the
 * surface normal at each point. Use this to reason about terrain elevation, e.g.
 * to place objects on the ground or test traversability.
 */
class HeightfieldQueryTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit HeightfieldQueryTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

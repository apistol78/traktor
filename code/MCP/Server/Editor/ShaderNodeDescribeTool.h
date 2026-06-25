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

namespace traktor::mcp
{

class Json;

/*! Tool: describe a single shader node type in depth.
 * \ingroup MCP
 *
 * Returns the node's pins (with commutative input groups), whether it is a
 * root/output node, and the captured GLSL emitter source for it — so the AI
 * can learn exactly what the node computes.
 */
class ShaderNodeDescribeTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;
};

}

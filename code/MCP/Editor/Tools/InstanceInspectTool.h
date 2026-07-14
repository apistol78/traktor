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

/*! Tool: inspect any database instance via reflection.
 * \ingroup MCP
 *
 * Returns the instance's type and a recursive description of its serialized
 * members (name, kind, type and value) using the Core Reflection mechanism,
 * so any ISerializable can be examined without type-specific knowledge.
 */
class InstanceInspectTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit InstanceInspectTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

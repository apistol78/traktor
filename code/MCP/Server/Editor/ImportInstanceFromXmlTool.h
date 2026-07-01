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

/*! Tool: replace an instance's object by deserializing a provided XML document.
 * \ingroup MCP
 *
 * Deserializes the supplied XML (the .xdi object form) via xml::XmlDeserializer
 * and commits it as the instance's object. Unlike set_instance_member's spec
 * system, the XML deserializer resolves ref= ids into shared object references,
 * so this can author assets whose editor representation needs object identity
 * (e.g. input mappings, shader graphs).
 */
class ImportInstanceFromXmlTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit ImportInstanceFromXmlTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

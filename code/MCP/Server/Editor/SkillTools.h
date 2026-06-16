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
#include "MCP/Server/IMcpPromptProvider.h"
#include "MCP/Server/IMcpTool.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mcp
{

class Json;

/*! Create (or overwrite) a skill instance in the database. */
class SkillCreateTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit SkillCreateTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;
	virtual std::wstring getDescription() const override final;
	virtual Ref< Json > getInputSchema() const override final;
	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

/*! List the skills stored in the database. */
class SkillListTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit SkillListTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;
	virtual std::wstring getDescription() const override final;
	virtual Ref< Json > getInputSchema() const override final;
	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

/*! Fetch the full body and parameters of a single skill. */
class SkillGetTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit SkillGetTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;
	virtual std::wstring getDescription() const override final;
	virtual Ref< Json > getInputSchema() const override final;
	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

/*! Publish (or unpublish) a skill, controlling whether it is exposed as a prompt. */
class SkillPublishTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit SkillPublishTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;
	virtual std::wstring getDescription() const override final;
	virtual Ref< Json > getInputSchema() const override final;
	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

/*! Exposes published skills through MCP "prompts/list"/"prompts/get". */
class SkillPromptProvider : public IMcpPromptProvider
{
	T_RTTI_CLASS;

public:
	explicit SkillPromptProvider(editor::IEditor* editor);

	virtual void listPrompts(Json* outPrompts) override final;
	virtual Ref< Json > getPrompt(const std::wstring& name, const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}

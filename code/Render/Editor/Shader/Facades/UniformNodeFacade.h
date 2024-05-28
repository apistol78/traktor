/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Editor/Shader/INodeFacade.h"

namespace traktor::ui
{

class Edit;
class INodeShape;

}

namespace traktor::render
{

class Uniform;

class UniformNodeFacade : public INodeFacade
{
	T_RTTI_CLASS;

public:
	UniformNodeFacade();

	virtual Ref< Node > createShaderNode(
		const TypeInfo* nodeType,
		editor::IEditor* editor
	) override final;

	virtual Ref< ui::Node > createEditorNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) override final;

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ui::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) override final;

	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ui::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) override final;

	virtual void setValidationIndicator(
		ui::Node* editorNode,
		bool validationSucceeded
	) override final;

private:
	Ref<ui::INodeShape > m_nodeShape;
	Ref< ui::Edit > m_edit;
	ui::Node* m_editEditorNode = nullptr;
	Uniform* m_editShaderNode = nullptr;
};

}

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

class INodeShape;

}

namespace traktor::render
{

class Texture;

class TextureNodeFacade : public INodeFacade
{
	T_RTTI_CLASS;

public:
	TextureNodeFacade();

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

	void updateThumb(editor::IEditor* editor, ui::GraphControl* graphControl, ui::Node* editorNode, Texture* texture) const;
};

}

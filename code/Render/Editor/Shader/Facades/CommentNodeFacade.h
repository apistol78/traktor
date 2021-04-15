#pragma once

#include "Render/Editor/Shader/INodeFacade.h"

namespace traktor
{
	namespace ui
	{

class INodeShape;

	}

	namespace render
	{

class CommentNodeFacade : public INodeFacade
{
	T_RTTI_CLASS;

public:
	CommentNodeFacade();

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
};

	}
}


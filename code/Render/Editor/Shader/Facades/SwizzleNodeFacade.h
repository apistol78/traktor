/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SwizzleNodeFacade_H
#define traktor_render_SwizzleNodeFacade_H

#include "Render/Editor/Shader/INodeFacade.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GraphControl;
class NodeShape;

		}
	}

	namespace render
	{

class SwizzleNodeFacade : public INodeFacade
{
	T_RTTI_CLASS;

public:
	SwizzleNodeFacade(ui::custom::GraphControl* graphControl);

	virtual Ref< Node > createShaderNode(
		const TypeInfo* nodeType,
		editor::IEditor* editor
	) T_OVERRIDE T_FINAL;

	virtual Ref< ui::custom::Node > createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) T_OVERRIDE T_FINAL;

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) T_OVERRIDE T_FINAL;

	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) T_OVERRIDE T_FINAL;

	virtual void setValidationIndicator(
		ui::custom::Node* editorNode,
		bool validationSucceeded
	) T_OVERRIDE T_FINAL;

private:
	Ref< ui::custom::NodeShape > m_nodeShape;
};

	}
}

#endif	// traktor_render_SwizzleNodeFacade_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_INodeFacade_H
#define traktor_render_INodeFacade_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
		namespace custom
		{

class GraphControl;
class Node;

		}
	}

	namespace render
	{

class Node;
class ShaderGraph;

class INodeFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Node > createShaderNode(
		const TypeInfo* nodeType,
		editor::IEditor* editor
	) = 0;

	virtual Ref< ui::custom::Node > createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	virtual void setValidationIndicator(
		ui::custom::Node* editorNode,
		bool validationSucceeded
	) = 0;
};

	}
}

#endif	// traktor_render_INodeFacade_H

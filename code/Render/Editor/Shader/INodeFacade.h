#pragma once

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

class GraphControl;
class Node;

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

	virtual Ref< ui::Node > createEditorNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ui::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ui::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	virtual void setValidationIndicator(
		ui::Node* editorNode,
		bool validationSucceeded
	) = 0;
};

	}
}


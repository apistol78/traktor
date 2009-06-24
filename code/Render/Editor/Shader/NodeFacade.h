#ifndef traktor_render_NodeFacade_H
#define traktor_render_NodeFacade_H

#include "Core/Object.h"

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

class NodeFacade : public Object
{
	T_RTTI_CLASS(NodeFacade)

public:
	virtual Node* createShaderNode(
		const Type* nodeType,
		editor::IEditor* editor
	) = 0;

	virtual ui::custom::Node* createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	) = 0;

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	) = 0;

	virtual void setValidationIndicator(
		ui::custom::Node* editorNode,
		bool validationSucceeded
	) = 0;
};

	}
}

#endif	// traktor_render_NodeFacade_H

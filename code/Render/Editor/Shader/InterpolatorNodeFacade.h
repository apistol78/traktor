#ifndef traktor_render_InterpolatorNodeFacade_H
#define traktor_render_InterpolatorNodeFacade_H

#include "Core/Heap/Ref.h"
#include "Render/Editor/Shader/NodeFacade.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class NodeShape;

		}
	}

	namespace render
	{

class InterpolatorNodeFacade : public NodeFacade
{
	T_RTTI_CLASS(InterpolatorNodeFacade)

public:
	InterpolatorNodeFacade();

	virtual Node* createShaderNode(
		const Type* nodeType,
		editor::Editor* editor
	);

	virtual ui::custom::Node* createEditorNode(
		editor::Editor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	);

	virtual void editShaderNode(
		editor::Editor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	);

	virtual void setValidationIndicator(
		ui::custom::Node* editorNode,
		bool validationSucceeded
	);

private:
	Ref< ui::custom::NodeShape > m_nodeShape;
};

	}
}

#endif	// traktor_render_InterpolatorNodeFacade_H

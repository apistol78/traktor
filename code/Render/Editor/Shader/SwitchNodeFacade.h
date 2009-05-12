#ifndef traktor_render_SwitchNodeFacade_H
#define traktor_render_SwitchNodeFacade_H

#include "Core/Heap/Ref.h"
#include "Render/Editor/Shader/NodeFacade.h"

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

class SwitchNodeFacade : public NodeFacade
{
	T_RTTI_CLASS(SwitchNodeFacade)

public:
	SwitchNodeFacade(ui::custom::GraphControl* graphControl);

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

#endif	// traktor_render_SwitchNodeFacade_H

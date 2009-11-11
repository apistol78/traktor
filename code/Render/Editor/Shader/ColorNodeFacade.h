#ifndef traktor_render_ColorNodeFacade_H
#define traktor_render_ColorNodeFacade_H

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

class ColorNodeFacade : public NodeFacade
{
	T_RTTI_CLASS(ColorNodeFacade)

public:
	ColorNodeFacade(ui::custom::GraphControl* graphControl);

	virtual Ref< Node > createShaderNode(
		const Type* nodeType,
		editor::IEditor* editor
	);

	virtual Ref< ui::custom::Node > createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	);

	virtual void editShaderNode(
		editor::IEditor* editor,
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

#endif	// traktor_render_ColorNodeFacade_H

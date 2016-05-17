#ifndef traktor_render_SwitchNodeFacade_H
#define traktor_render_SwitchNodeFacade_H

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
	T_RTTI_CLASS;

public:
	SwitchNodeFacade(ui::custom::GraphControl* graphControl);

	virtual Ref< Node > createShaderNode(
		const TypeInfo* nodeType,
		editor::IEditor* editor
	) T_OVERRIDE T_FINAL;

	virtual Ref< ui::custom::Node > createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	) T_OVERRIDE T_FINAL;

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		Node* shaderNode
	) T_OVERRIDE T_FINAL;

	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
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

#endif	// traktor_render_SwitchNodeFacade_H

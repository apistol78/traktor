#ifndef traktor_render_InterpolatorNodeFacade_H
#define traktor_render_InterpolatorNodeFacade_H

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
	T_RTTI_CLASS;

public:
	InterpolatorNodeFacade();

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

#endif	// traktor_render_InterpolatorNodeFacade_H

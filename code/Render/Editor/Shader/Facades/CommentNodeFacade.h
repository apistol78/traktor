#ifndef traktor_render_CommentNodeFacade_H
#define traktor_render_CommentNodeFacade_H

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

class CommentNodeFacade : public INodeFacade
{
	T_RTTI_CLASS;

public:
	CommentNodeFacade(ui::custom::GraphControl* graphControl);

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

#endif	// traktor_render_CommentNodeFacade_H

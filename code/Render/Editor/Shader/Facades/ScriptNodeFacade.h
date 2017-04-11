#ifndef traktor_render_ScriptNodeFacade_H
#define traktor_render_ScriptNodeFacade_H

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

class ShaderGraphEditorPage;

class ScriptNodeFacade : public INodeFacade
{
	T_RTTI_CLASS;

public:
	ScriptNodeFacade(ShaderGraphEditorPage* page, ui::custom::GraphControl* graphControl);

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
	ShaderGraphEditorPage* m_page;
	Ref< ui::custom::NodeShape > m_nodeShape;
};

	}
}

#endif	// traktor_render_ScriptNodeFacade_H
